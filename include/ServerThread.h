#ifndef __SERVERTHREAD_H__
#define __SERVERTHREAD_H__

#include <condition_variable>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <map>
#include <vector>
#include <string>

#include "Messages.h"
#include "ServerSocket.h"
#include "CustomerRecords.h"
#include "StateMachineLog.h"
#include "ReplicationStub.h"

class ServerStub;

struct AdminResult { int op_id; int admin_id; };
struct AdminRequest {
  RobotInfo robot;
  std::promise<AdminResult> prom;
};

struct PeerInfo {
  int id;
  std::string ip;
  int port;
};

class RobotFactory {
private:
  std::queue<std::unique_ptr<AdminRequest>> arq;
  std::mutex arq_lock;
  std::condition_variable arq_cv;

  CustomerRecords records_;
  StateMachineLog log_;

  struct FactoryState {
    std::mutex m;
    int last_index = -1;
    int committed_index = -1;
    int primary_id = -1;
    int factory_id = -1;
  } state_;

  std::vector<PeerInfo> peers_;
  std::map<int, std::unique_ptr<ReplicationStub>> pfa_conns_;
  bool pfa_connected_{false};

  RobotInfo CreateRobotFromRequest(const CustomerRequest& req, int engineer_id);
  AdminResult SubmitAdminUpdate(const RobotInfo& robot);

  void EnsurePfaConnectionsIfNeededUnlocked();
  void RefreshAndRepairPeersUnlocked();              
  void SyncPeerLogsUnlocked(int peer_id);            

  void IFALoop(ServerStub& stub);

public:
  void Configure(int factory_id, const std::vector<PeerInfo>& peers);
  void EngineerThread(std::unique_ptr<ServerSocket> socket, int id);
  void AdminThread(int id);
};

#endif
