#ifndef __CLIENT_STUB_H__
#define __CLIENT_STUB_H__

#include <string>
#include "ClientSocket.h"
#include "Messages.h"

class ClientStub {
private:
  ClientSocket socket;
  bool healthy_ = true;   

public:
  ClientStub();
  int Init(std::string ip, int port);

  int IdentifyAsCustomer();

  RobotInfo Order(const CustomerRequest& req);

  CustomerRecord ReadRecord(const CustomerRequest& req);

  bool Healthy() const { return healthy_; }
};

#endif
