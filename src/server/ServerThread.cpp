#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include <algorithm>

#include "ServerThread.h"
#include "ServerStub.h"

void RobotFactory::Configure(int factory_id, const std::vector<PeerInfo>& peers) {
  {
    std::lock_guard<std::mutex> lg(state_.m);
    state_.factory_id = factory_id;
    state_.primary_id = -1;
    state_.last_index = -1;
    state_.committed_index = -1;
  }
  peers_ = peers;
}

RobotInfo RobotFactory::CreateRobotFromRequest(const CustomerRequest& req, int engineer_id) {
  RobotInfo robot;
  robot.CopyRequest(req);
  robot.SetEngineerId(engineer_id);
  robot.SetAdminId(-1);
  return robot;
}

AdminResult RobotFactory::SubmitAdminUpdate(const RobotInfo& robot) {
  std::promise<AdminResult> prom;
  std::future<AdminResult> fut = prom.get_future();

  std::unique_ptr<AdminRequest> req(new AdminRequest);
  req->robot = robot;
  req->prom = std::move(prom);

  { std::lock_guard<std::mutex> lg(arq_lock); arq.push(std::move(req)); }
  arq_cv.notify_one();

  return fut.get();
}

void RobotFactory::EnsurePfaConnectionsIfNeededUnlocked() {
  if (pfa_connected_) return;
  for (const auto& p : peers_) {
    if (p.id == state_.factory_id) continue;
    std::unique_ptr<ReplicationStub> stub(new ReplicationStub());
    if (stub->ConnectAndIdentify(p.ip, p.port)) {
      pfa_conns_[p.id] = std::move(stub);
    }
  }
  pfa_connected_ = true;
}

void RobotFactory::RefreshAndRepairPeersUnlocked() {
  for (const auto& p : peers_) {
    if (p.id == state_.factory_id) continue;
    if (pfa_conns_.count(p.id)) continue;  
    std::unique_ptr<ReplicationStub> stub(new ReplicationStub());
    if (stub->ConnectAndIdentify(p.ip, p.port)) {
      pfa_conns_[p.id] = std::move(stub);
      SyncPeerLogsUnlocked(p.id);
    }
  }
}


void RobotFactory::SyncPeerLogsUnlocked(int peer_id) {
  auto it = pfa_conns_.find(peer_id);
  if (it == pfa_conns_.end()) return;
  ReplicationStub* stub = it->second.get();

  // int last_idx_snapshot, committed_snapshot, pfa_id_snapshot;
  int last_idx_snapshot, pfa_id_snapshot;

  {
    std::lock_guard<std::mutex> lg(state_.m);
    last_idx_snapshot = state_.last_index;
    // committed_snapshot = state_.committed_index;
    pfa_id_snapshot = state_.factory_id;
  }

  for (int i = 0; i <= last_idx_snapshot; ++i) {
    MapOp op = log_.At(static_cast<size_t>(i));
    ReplicationRequest rr(pfa_id_snapshot, /*commit up to*/ i-1, /*write at*/ i, op);
    ReplicationReply rep;
    if (!stub->Replicate(rr, &rep)) {
      pfa_conns_.erase(peer_id);
      return;
    }
  }
}

void RobotFactory::EngineerThread(std::unique_ptr<ServerSocket> socket, int id) {
  ServerStub stub; stub.Init(std::move(socket));

  RoleType role = stub.ReceiveRole();

  if (role == RoleType::PFA) {
    IFALoop(stub);
    return;
  }

  int engineer_id = id;
  while (true) {
    CustomerRequest req = stub.ReceiveRequest();
    if (!req.IsValid()) break;

    if (req.GetRequestType() == 1) {
      RobotInfo robot = CreateRobotFromRequest(req, engineer_id);
      AdminResult res = SubmitAdminUpdate(robot);
      robot.SetAdminId(res.admin_id);
      stub.ShipRobot(robot);
    } else if (req.GetRequestType() == 2) {
      int last = -1;
      records_.Read(req.GetCustomerId(), &last);
      CustomerRecord rec;
      if (last >= 0) rec.Set(req.GetCustomerId(), last);
      else rec.Set(-1, -1);
      stub.ReturnRecord(rec);
    } else {
      break;
    }
  }
}

void RobotFactory::IFALoop(ServerStub& stub) {
  while (true) {
    ReplicationRequest rr = stub.ReceiveReplication();
    if (rr.GetPfaId() == -1) {
      std::lock_guard<std::mutex> lg(state_.m);
      state_.primary_id = -1;
      break;
    }

    {
      std::lock_guard<std::mutex> lg(state_.m);
      if (state_.primary_id != rr.GetPfaId()) {
        state_.primary_id = rr.GetPfaId();
      }

      int lidx = rr.GetLastIndex();
      log_.WriteAt(static_cast<size_t>(lidx), rr.GetOp());
      if (lidx > state_.last_index) state_.last_index = lidx;

      int target_commit = rr.GetCommittedIndex();
      int from = state_.committed_index + 1;
      int to = std::min(target_commit, state_.last_index);
      for (int i = from; i <= to; ++i) {
        MapOp op = log_.At(static_cast<size_t>(i));
        if (op.opcode == 1) {
          records_.Update(op.arg1, op.arg2);
        }
      }
      if (target_commit > state_.committed_index) state_.committed_index = target_commit;
    }

    ReplicationReply rep(/*backup_id*/ state_.factory_id, /*ok*/1,
                         /*last_index*/ state_.last_index,
                         /*committed*/ state_.committed_index);
    stub.SendReplicationReply(rep);
  }
}

void RobotFactory::AdminThread(int id) {
  const int admin_id = id;
  std::unique_lock<std::mutex> ul(arq_lock, std::defer_lock);

  while (true) {
    ul.lock();
    if (arq.empty()) {
      arq_cv.wait(ul, [this]{ return !arq.empty(); });
    }
    auto req = std::move(arq.front());
    arq.pop();
    ul.unlock();

    {
      std::lock_guard<std::mutex> lg(state_.m);
      if (state_.primary_id != state_.factory_id) {
        state_.primary_id = state_.factory_id;
        EnsurePfaConnectionsIfNeededUnlocked();

        RefreshAndRepairPeersUnlocked();

        if (state_.committed_index < state_.last_index) {
          for (int i = state_.committed_index + 1; i <= state_.last_index; ++i) {
            MapOp op = log_.At(static_cast<size_t>(i));
            if (op.opcode == 1) {
              records_.Update(op.arg1, op.arg2);
            }
          }
          state_.committed_index = state_.last_index;
        }
      } else {
        RefreshAndRepairPeersUnlocked();
      }
    }

    MapOp op { 1, req->robot.GetCustomerId(), req->robot.GetOrderNumber() };

    int new_index;
    int current_committed;
    {
      std::lock_guard<std::mutex> lg(state_.m);
      new_index = state_.last_index + 1;
      log_.WriteAt(static_cast<size_t>(new_index), op);
      state_.last_index = new_index;
      current_committed = state_.committed_index;
    }

    ReplicationRequest rr(state_.factory_id, current_committed, new_index, op);
    // bool all_ok = true;

    for (auto it = pfa_conns_.begin(); it != pfa_conns_.end(); ) {
      ReplicationReply reply;
      bool ok = it->second->Replicate(rr, &reply);
      if (!ok || reply.GetOk()!=1) {
        it = pfa_conns_.erase(it);
        continue;
      } else {
        ++it;
      }
    }

    {
      std::lock_guard<std::mutex> lg(state_.m);
      if (op.opcode == 1) {
        records_.Update(op.arg1, op.arg2);
      }
      state_.committed_index = state_.last_index;
    }

    AdminResult r{ new_index, admin_id };
    req->prom.set_value(r);
  }
}
