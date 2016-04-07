#ifndef _TESTGLOBABLS_HPP_
#define _TESTGLOBABLS_HPP_

#include <SFML/Window.hpp>
#include <engine/GameWorld.hpp>
#include <engine/GameData.hpp>
#include <core/Logger.hpp>
#include <glm/gtx/string_cast.hpp>

#define ENV_GAME_PATH_NAME ("OPENRW_GAME_PATH")

std::ostream& operator<<( std::ostream& stream, glm::vec3 const& v );

namespace boost { namespace test_tools {
template<>
struct print_log_value<glm::vec3> {
	void operator()( std::ostream& s , glm::vec3 const& v )
	{
		s << glm::to_string(v);
	}
};
}}

namespace boost { namespace test_tools {
template<>
struct print_log_value<std::nullptr_t> {
	void operator()( std::ostream& s , std::nullptr_t )
	{
		s << "nullptr";
	}
};
}}

class Global
{
public:
	sf::Window wnd;
	GameData* d;
	GameWorld* e;
	Logger log;
	WorkContext work;
	
	Global() {
		wnd.create(sf::VideoMode(640, 360), "Testing");
		d = new GameData(&log, &work, getGamePath());
		e = new GameWorld(&log, &work, d);

		e->data->loadIMG("/models/gta3");
		e->data->loadIMG("/anim/cuts");
		e->data->load();
		for(std::map<std::string, std::string>::iterator it = e->data->ideLocations.begin();
			it != e->data->ideLocations.end();
			++it) {
			d->loadObjects(it->second);
		}

		e->dynamicsWorld->setGravity(btVector3(0.f, 0.f, 0.f));

		while( ! e->_work->isEmpty() ) {
			std::this_thread::yield();
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
