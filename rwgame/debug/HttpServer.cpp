#include "HttpServer.hpp"

#include <SFML/Network.hpp>

#include <iostream>

HttpServer::HttpServer(RWGame* game, GameWorld* world)
	: game(game), world(world)
{}

void HttpServer::run()
{
	listener.listen(8091);
	listener.reuse();

	std::cout << "STARTING HTTP SERVER" << std::endl;

	sf::TcpSocket client;
	if (listener.accept(client) == sf::Socket::Done) {
		std::cout << "New connection from "
			<< client.getRemoteAddress() << ":" << client.getRemotePort()
			<< std::endl;

		char buf[1024];
		size_t received;
		client.receive(buf, 1023, received);
		buf[received] = '\0';
		std::cout << "Got " << received << " bytes: " << buf << std::endl;

		#define DATASEND(x) x, strlen(x)
		client.send(DATASEND("HTTP/1.1 200 OK\n\n<h1>HELLO FROM OPENRW</h1>"));
	}

	listener.close();
}
