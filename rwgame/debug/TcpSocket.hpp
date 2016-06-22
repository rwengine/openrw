#pragma once

#include <string>
#include <netinet/in.h>


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

	std::string getRemoteAddress() const;
	short getRemotePort() const;

private:
	int sock;
	sockaddr_in addr;
};
