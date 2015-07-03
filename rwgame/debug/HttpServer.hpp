#pragma once

#include "../RWGame.hpp"
#include <engine/GameWorld.hpp>

#include <SFML/Network/TcpListener.hpp>
#ifdef SFML_SYSTEM_WINDOWS
	#include <winsock2.h>
#else
	#include <sys/socket.h>
#endif

class ReuseableListener : public sf::TcpListener
{
	public:
	void reuse() {
		char reuse = 1;
		setsockopt(getHandle(), SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	}
};

class HttpServer
{
public:
	HttpServer(RWGame* game, GameWorld* world);
	void run();

    void handleBreakpoint(const SCMBreakpoint& bp);
private:
	ReuseableListener listener;
	RWGame* game;
	GameWorld* world;
    bool paused;

    std::string dispatch(std::string method, std::string path);
    std::string getState() const;
};
