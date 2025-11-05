#ifndef __CLIENT_THREAD_H__
#define __CLIENT_THREAD_H__

#include <string>
#include "ClientStub.h"
#include "ClientTimer.h"

class ClientThreadClass {
  int customer_id;
  int num_orders;
  int request_type;
  ClientStub stub;
  ClientTimer timer;

  void DoOrderLoop();
  void DoReadOwnLoop();
  void DoScanAndPrintLoop();

public:
  ClientThreadClass();
  void ThreadBody(std::string ip, int port, int id, int orders, int type);
  ClientTimer GetTimer();
};

#endif
