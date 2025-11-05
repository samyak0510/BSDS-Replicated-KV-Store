#include "ServerStub.h"
#include <arpa/inet.h>
#include <cstring>

ServerStub::ServerStub() {}

void ServerStub::Init(std::unique_ptr<ServerSocket> s) {
  socket = std::move(s);
}

RoleType ServerStub::ReceiveRole() {
  int32_t v = 0;
  if (!socket->Recv(reinterpret_cast<char*>(&v), sizeof(v), 0)) {
    return static_cast<RoleType>(0); // invalid
  }
  v = ntohl(v);
  return static_cast<RoleType>(v);
}

CustomerRequest ServerStub::ReceiveRequest() {
  CustomerRequest req;                // default invalid
  char buf[16];                       // 12 bytes needed
  if (!socket->Recv(buf, req.Size(), 0)) {
    return CustomerRequest();         // invalid; caller will close the thread/conn
  }
  req.Unmarshal(buf);
  return req;
}

int ServerStub::ShipRobot(const RobotInfo& info) {
  char buf[32];                       // 20 bytes needed
  RobotInfo tmp = info;
  tmp.Marshal(buf);
  return socket->Send(buf, tmp.Size(), 0);
}

int ServerStub::ReturnRecord(const CustomerRecord& rec) {
  char buf[16];                       // 8 bytes needed
  CustomerRecord tmp = rec;
  tmp.Marshal(buf);
  return socket->Send(buf, tmp.Size(), 0);
}

ReplicationRequest ServerStub::ReceiveReplication() {
  ReplicationRequest rr;              // default has pfa_id=-1
  char buf[32];                       // 24 bytes needed
  // We MUST use rr.Size() here, which you've set to 24 bytes.
  if (!socket->Recv(buf, rr.Size(), 0)) {
    return ReplicationRequest();      // pfa_id stays -1 -> IFA loop will exit
  }
  rr.Unmarshal(buf);
  return rr;
}

int ServerStub::SendReplicationReply(const ReplicationReply& rep) {
  char buf[16];                       // 16 bytes needed
  ReplicationReply tmp = rep;
  tmp.Marshal(buf);
  return socket->Send(buf, tmp.Size(), 0);
}