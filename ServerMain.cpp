#include <iostream>
#include <thread>
#include <vector>
#include <string>

#include "ServerSocket.h"
#include "ServerThread.h"

static void Usage(const char* prog) {
  std::cout << "Usage:\n"
            << prog << " [port] [factory_id] [#peers] (repeat [peer_id] [peer_ip] [peer_port])\n"
            << "Example (ID=0): " << prog << " 12345 0 2 1 22.22.22.22 12345 2 33.33.33.33 12345\n";
}

int main(int argc, char *argv[]) {
  if (argc < 4) { Usage(argv[0]); return 0; }

  int argi = 1;
  int port = atoi(argv[argi++]);
  int my_id = atoi(argv[argi++]);
  int npeers = atoi(argv[argi++]);

  if (argc != 4 + npeers*3) { Usage(argv[0]); return 0; }

  std::vector<PeerInfo> peers;
  peers.push_back({ my_id, std::string("127.0.0.1"), port });
  for (int i=0;i<npeers;i++) {
    int pid = atoi(argv[argi++]);
    std::string pip = argv[argi++];
    int pport = atoi(argv[argi++]);
    peers.push_back({ pid, pip, pport });
  }

  ServerSocket listen_sock;
  if (!listen_sock.Init(port)) {
    std::cout << "Socket initialization failed\n";
    return 0;
  }

  RobotFactory factory;
  factory.Configure(my_id, peers);

  std::thread admin_thread(&RobotFactory::AdminThread, &factory, /*id=*/0);

  std::vector<std::thread> threads;
  while (true) {
    std::unique_ptr<ServerSocket> s = listen_sock.Accept();
    if (!s) {
      // Transient accept error: don’t bring the server down. Try again.
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      continue;
    }
    threads.emplace_back(&RobotFactory::EngineerThread, &factory, std::move(s),
                         /*engineer_id*/ static_cast<int>(threads.size()+1));
  }

  admin_thread.join();
  for (auto& t : threads) t.join();
  return 0;
}
