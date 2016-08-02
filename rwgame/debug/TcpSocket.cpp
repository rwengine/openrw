#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "TcpSocket.hpp"


TcpSocket::TcpSocket() :
	sock(-1), addr()
{
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		perror("TcpSocket: Could not create socket");
}


bool TcpSocket::bind(short port)
{
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	if (::bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr))) {
		perror("TcpSocket: Could not bind address to socket");
		return false;
	}

	if (::listen(sock, 1)) {
		perror("TcpSocket: Could not listen for connections");
		return false;
	}

	return true;
}


bool TcpSocket::listen(TcpSocket& client)
{
	socklen_t len = sizeof(client.addr);
	client.sock = accept(sock, reinterpret_cast<sockaddr*>(&client.addr), &len);

	return client.sock != -1;
}


void TcpSocket::recv(std::string& out, size_t len)
{
	std::vector<char> buffer(len);
	int ret = read(sock, buffer.data(), len);

	if (ret != -1)
		out = { buffer.begin(), buffer.begin()+ret };
	else
		out = "";
}


size_t TcpSocket::send(const std::string& str)
{
	int ret = write(sock, str.c_str(), str.length());
	return ret != -1 ? ret : 0;
}


void TcpSocket::disconnect()
{
	#ifndef RW_WINDOWS
	  close(sock);
	#else
	  closesocket(sock);
	#endif
	sock = -1;
}


char* TcpSocket::getRemoteAddress() const
{
	return inet_ntoa(addr.sin_addr);
}


short TcpSocket::getRemotePort() const
{
	return ntohs(addr.sin_port);
}
