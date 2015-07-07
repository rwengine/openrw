#pragma once

#include "../RWGame.hpp"
#include <engine/GameWorld.hpp>

#include <SFML/Network/TcpListener.hpp>
#ifdef SFML_SYSTEM_WINDOWS
	#include <winsock2.h>
#else
	#include <sys/socket.h>
#endif

class HttpServer
{
public:
	HttpServer(RWGame* game, GameWorld* world);
	void run();

    void handleBreakpoint(const SCMBreakpoint& bp);
private:
    sf::TcpListener listener;
	RWGame* game;
	GameWorld* world;
    bool paused;
    const SCMBreakpoint* lastBreakpoint;

    std::string dispatch(std::string method, std::string path);
    std::string getState() const;
};
