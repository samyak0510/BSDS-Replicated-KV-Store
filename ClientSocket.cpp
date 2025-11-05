#include <string.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <fcntl.h>       
#include <sys/select.h>  
#include <errno.h>       
#include <unistd.h>      

#include "ClientSocket.h"

static int do_connect_with_timeout(int& fd, const std::string& ip, int port) {
    struct sockaddr_in addr;
    fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return 0;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    addr.sin_port = htons(port);

    long arg = fcntl(fd, F_GETFL, NULL);
    arg |= O_NONBLOCK;
    fcntl(fd, F_SETFL, arg);

    int connect_res = ::connect(fd, (struct sockaddr *) &addr, sizeof(addr));
    if (connect_res < 0) {
        if (errno != EINPROGRESS) {
            ::close(fd); fd = -1;
            return 0;
        }
    }

    if (connect_res != 0) {
        fd_set read_set, write_set;
        FD_ZERO(&read_set);
        FD_ZERO(&write_set);
        FD_SET(fd, &read_set);
        FD_SET(fd, &write_set);
        
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;

        int sel_res = select(fd + 1, &read_set, &write_set, NULL, &timeout);
        if (sel_res <= 0) {
            ::close(fd); fd = -1;
            return 0; 
        }

        int so_error;
        socklen_t len = sizeof(so_error);
        getsockopt(fd, SOL_SOCKET, SO_ERROR, &so_error, &len);
        if (so_error != 0) {
            ::close(fd); fd = -1;
            return 0;
        }
    }
    
    arg = fcntl(fd, F_GETFL, NULL);
    arg &= ~O_NONBLOCK;
    fcntl(fd, F_SETFL, arg);

    return 1; 
}

static int do_connect_blocking(int& fd, bool& inited, const std::string& ip, int port, bool quiet) {
  if (inited) return 0;
  struct sockaddr_in addr;
  fd = ::socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    if (!quiet) perror("ERROR: failed to create a socket");
    return 0;
  }
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(ip.c_str());
  addr.sin_port = htons(port);
  if (::connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    if (!quiet) perror("ERROR: failed to connect");
    ::close(fd);
    fd = -1;
    return 0;
  }
  inited = true;
  return 1;
}

int ClientSocket::Init(std::string ip, int port) {
  return do_connect_blocking(fd_, is_initialized_, ip, port, /*quiet=*/false);
}

int ClientSocket::InitQuiet(const std::string& ip, int port) {
    if (is_initialized_) return 0;
    int ok = do_connect_with_timeout(fd_, ip, port);
    if (ok) is_initialized_ = true;
    return ok;
}

int ClientSocket::SetTimeoutMs(int ms) {
  if (fd_ < 0) return 0;
  struct timeval tv;
  tv.tv_sec  = ms / 1000;
  tv.tv_usec = (ms % 1000) * 1000;
  if (::setsockopt(fd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) return 0;
  if (::setsockopt(fd_, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0) return 0;
  return 1;
}