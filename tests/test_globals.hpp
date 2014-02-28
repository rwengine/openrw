#ifndef _TESTGLOBABLS_HPP_
#define _TESTGLOBABLS_HPP_

#include <SFML/Window.hpp>
#include <engine/GameWorld.hpp>

// Many tests require OpenGL be functional, seems like a reasonable solution.

class Global
{
public:
	sf::Window wnd;
	GameWorld* e;
	
	Global() {
		wnd.create(sf::VideoMode(640, 360), "Testing");
		glewExperimental = GL_TRUE;
		glewInit();
		e = new GameWorld("test_data");

		e->gameData.loadIMG("/models/gta3");
		e->load();
		for(std::map<std::string, std::string>::iterator it = e->gameData.ideLocations.begin();
			it != e->gameData.ideLocations.end();
			++it) {
			e->defineItems(it->second);
		}
	}

	~Global() {
		wnd.close();
		delete e;
	}
	
	static Global& get()
	{
		static Global g;
		return g;
	}
};

#endif 
