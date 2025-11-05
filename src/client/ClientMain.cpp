#include <array>
#include <iostream>
#include <iomanip>
#include <thread>
#include <vector>

#include "ClientSocket.h"
#include "ClientThread.h"
#include "ClientTimer.h"

int main(int argc, char *argv[]) {
  std::string ip;
  int port;
  int num_customers;
  int num_orders;
  int request_type;

  if (argc < 6) {
    std::cout << "not enough arguments" << std::endl;
    std::cout << argv[0] << " [ip] [port #] [# customers] [# orders] [request type (1=order,2=read,3=scan)]" << std::endl;
    return 0;
  }

  ip = argv[1];
  port = atoi(argv[2]);
  num_customers = atoi(argv[3]);
  num_orders = atoi(argv[4]);
  request_type = atoi(argv[5]);

  ClientTimer timer;
  std::vector<std::unique_ptr<ClientThreadClass>> client_vector;
  std::vector<std::thread> thread_vector;


  for (int i = 0; i < num_customers; i++) {
    std::unique_ptr<ClientThreadClass> pc(new ClientThreadClass());
    std::thread client_thread(&ClientThreadClass::ThreadBody, pc.get(),
                              ip, port, i, num_orders, request_type);
    client_vector.push_back(std::move(pc));
    thread_vector.push_back(std::move(client_thread));
  }

  for (auto& th : thread_vector) th.join();

  for (auto& cls : client_vector) {
    timer.Merge(cls->GetTimer());
  }

  timer.PrintStats();

  return 0;
}
