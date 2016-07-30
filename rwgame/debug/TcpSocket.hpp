#pragma once

#include <string>
#ifndef RW_WINDOWS
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#else
#include <ws2tcpip.h>
#endif

class TcpSocket
{
public:
	TcpSocket();
	~TcpSocket() { disconnect(); }

	bool bind(short port);
	bool listen(TcpSocket& client);

	void recv(std::string& out, size_t len);
	size_t send(const std::string& str);
	void disconnect();

	char* getRemoteAddress() const;
	short getRemotePort() const;

private:
	int sock;
	sockaddr_in addr;
};
