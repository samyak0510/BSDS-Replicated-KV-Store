#include "ClientStub.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>

ClientStub::ClientStub() {}
int ClientStub::Init(std::string ip, int port) {
  int ok = socket.Init(ip, port);
  healthy_ = (ok != 0);
  return ok;
}

int ClientStub::IdentifyAsCustomer() {
  int32_t v = htonl(static_cast<int32_t>(RoleType::CUSTOMER));
  int ok = socket.Send(reinterpret_cast<char*>(&v), sizeof(v), 0);
  healthy_ = healthy_ && ok;
  return ok;
}

RobotInfo ClientStub::Order(const CustomerRequest& req) {
  RobotInfo info; char b[32]; req.Marshal(b);
  int sz = req.Size();
  if (!socket.Send(b, sz, 0)) { healthy_ = false; return info; }
  sz = info.Size();
  if (!socket.Recv(b, sz, 0)) { healthy_ = false; return info; }
  info.Unmarshal(b);
  return info;
}

CustomerRecord ClientStub::ReadRecord(const CustomerRequest& req) {
  CustomerRecord rec; char b[32]; req.Marshal(b);
  int sz = req.Size();
  if (!socket.Send(b, sz, 0)) { healthy_ = false; return rec; }
  sz = rec.Size();
  if (!socket.Recv(b, sz, 0)) { healthy_ = false; return rec; }
  rec.Unmarshal(b);
  return rec;
}
