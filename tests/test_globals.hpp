#ifndef _TESTGLOBABLS_HPP_
#define _TESTGLOBABLS_HPP_

#include <SFML/Window.hpp>
#include <engine/GameWorld.hpp>

#define ENV_GAME_PATH_NAME ("OPENRW_GAME_PATH")

class Global
{
public:
	sf::Window wnd;
	GameWorld* e;
	
	Global() {
		wnd.create(sf::VideoMode(640, 360), "Testing");
		glewExperimental = GL_TRUE;
		glewInit();
		e = new GameWorld(getGamePath());

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

	static std::string getGamePath()
	{
		// TODO: Is this "the way to do it" on windows.
		auto v = getenv(ENV_GAME_PATH_NAME);
		return v ? v : "";
	}
	
	static Global& get()
	{
		static Global g;
		return g;
	}
};

#endif 
