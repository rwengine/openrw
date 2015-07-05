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
    void create() {
		char reuse = 1;
        sf::SocketHandle handle = socket(PF_INET, SOCK_STREAM, 0);
        if( setsockopt(handle, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0 )
        {
            std::cerr << "Failed to set socket SO_REUSEADDR: errno = " << errno << std::endl;
        }
        Socket::create(handle);
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
