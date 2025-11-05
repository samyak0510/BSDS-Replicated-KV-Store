#ifndef __SERVER_STUB_H__
#define __SERVER_STUB_H__

#include <memory>
#include "ServerSocket.h"
#include "Messages.h"

class ServerStub {
private:
  std::unique_ptr<ServerSocket> socket;

public:
  ServerStub();
  void Init(std::unique_ptr<ServerSocket> socket);

  RoleType ReceiveRole();

  CustomerRequest ReceiveRequest();
  int ShipRobot(const RobotInfo& info);
  int ReturnRecord(const CustomerRecord& rec);

  ReplicationRequest ReceiveReplication();
  int SendReplicationReply(const ReplicationReply& rep);
};

#endif
