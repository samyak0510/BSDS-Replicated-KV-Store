#ifndef __CLIENTSOCKET_H__
#define __CLIENTSOCKET_H__

#include <string>

#include "Socket.h"


class ClientSocket: public Socket {
public:
	ClientSocket() {}
	~ClientSocket() {}

	int Init(std::string ip, int port);
	int InitQuiet(const std::string& ip, int port);
	int SetTimeoutMs(int ms);
};


#endif // end of #ifndef __CLIENTSOCKET_H__
