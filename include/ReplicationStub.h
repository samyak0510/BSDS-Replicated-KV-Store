#ifndef __REPLICATION_STUB_H__
#define __REPLICATION_STUB_H__

#include <string>
#include <cstdint>
#include <arpa/inet.h>   

#include "ClientSocket.h"
#include "Messages.h"

class ReplicationStub {
private:
  ClientSocket socket;
  bool ok_{false};

public:
  ReplicationStub() = default;

  bool ConnectAndIdentify(const std::string& ip, int port) {
    if (!socket.InitQuiet(ip, port)) return false;
    socket.SetTimeoutMs(300);
    int32_t v = htonl(static_cast<int32_t>(RoleType::PFA));
    ok_ = socket.Send(reinterpret_cast<char*>(&v), sizeof(v), 0) != 0;
    return ok_;
  }

  bool Replicate(const ReplicationRequest& req, ReplicationReply* out) {
    if (!ok_) return false;

    const int req_size = req.Size();
    char buf[32];
    req.Marshal(buf);
    if (!socket.Send(buf, req_size, 0)) return false;

    char rbuf[16];
    if (!socket.Recv(rbuf, sizeof(rbuf), 0)) return false;
    if (out) {
      ReplicationReply rep; rep.Unmarshal(rbuf);
      *out = rep;
    }
    return true;
  }
};

#endif
