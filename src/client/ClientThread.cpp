#include "ClientThread.h"
#include "Messages.h"
#include <iostream>

ClientThreadClass::ClientThreadClass() : customer_id(0), num_orders(0), request_type(1) {}

void ClientThreadClass::ThreadBody(std::string ip, int port, int id, int orders, int type) {
  customer_id = id;
  num_orders = orders;
  request_type = type;

  if (!stub.Init(ip, port)) {
    std::cout << "Thread " << customer_id << " failed to connect" << std::endl;
    return;
  }
  if (!stub.IdentifyAsCustomer()) {
    std::cout << "Handshake failed for " << customer_id << std::endl;
    return;
  }

  if (request_type == 1) DoOrderLoop();
  else if (request_type == 2) DoReadOwnLoop();
  else if (request_type == 3) DoScanAndPrintLoop();
}

void ClientThreadClass::DoOrderLoop() {
  for (int i = 0; i < num_orders; i++) {
    if (!stub.Healthy()) break;
    CustomerRequest req; req.Set(customer_id, /*order_number=*/i, /*request_type=*/1);
    timer.Start(); RobotInfo robot = stub.Order(req); timer.EndAndMerge();
    if (!stub.Healthy()) {
      std::cout << "Server failed during order; exiting client thread " << customer_id << "\n";
      break;
    }
    if (!robot.IsValid()) { break; }
  }
}

void ClientThreadClass::DoReadOwnLoop() {
  for (int i = 0; i < num_orders; i++) {
    if (!stub.Healthy()) break;
    CustomerRequest req; req.Set(customer_id, -1, 2);
    timer.Start(); (void)stub.ReadRecord(req); timer.EndAndMerge();
    if (!stub.Healthy()) {
      std::cout << "Server failed during read; exiting client thread " << customer_id << "\n";
      break;
    }
  }
}

void ClientThreadClass::DoScanAndPrintLoop() {
  for (int cid = 0; cid <= num_orders; cid++) {
    if (!stub.Healthy()) break;
    CustomerRequest req; req.Set(cid, -1, 2);

    timer.Start();  
    CustomerRecord rec = stub.ReadRecord(req);
    timer.EndAndMerge();
    if (!stub.Healthy()) {
      std::cout << "Server failed during scan; exiting.\n";
      break;
    }
    if (rec.IsValid()) { std::cout << rec.GetCustomerId() << "\t" << rec.GetLastOrder() << std::endl; }
  }
}

ClientTimer ClientThreadClass::GetTimer() { return timer; }
