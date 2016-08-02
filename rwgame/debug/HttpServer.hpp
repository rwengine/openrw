#pragma once

#include "../RWGame.hpp"
#include <engine/GameWorld.hpp>
#include "TcpSocket.hpp"


class HttpServer
{
public:
	HttpServer(RWGame* game, GameWorld* world);
	void run();

    void handleBreakpoint(const SCMBreakpoint& bp);
private:
	TcpSocket socket;
	RWGame* game;
	GameWorld* world;
    bool paused;
    const SCMBreakpoint* lastBreakpoint;

    std::string dispatch(const std::string& method, const std::string& path);
    std::string getState() const;
};
