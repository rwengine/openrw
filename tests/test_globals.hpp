#ifndef _TESTGLOBABLS_HPP_
#define _TESTGLOBABLS_HPP_

#include <SFML/Window.hpp>
#include <engine/GameWorld.hpp>
#include <engine/GameData.hpp>
#include <engine/GameState.hpp>
#include <core/Logger.hpp>
#include <glm/gtx/string_cast.hpp>

#define ENV_GAME_PATH_NAME ("OPENRW_GAME_PATH")

std::ostream& operator<<( std::ostream& stream, glm::vec3 const& v );

// Boost moved the print_log_value struct in version 1.59
// TODO: use another testing library
#if BOOST_VERSION >= 105900
	#define BOOST_NS_MAGIC namespace tt_detail {
	#define BOOST_NS_MAGIC_CLOSING }
#else
	#define BOOST_NS_MAGIC
	#define BOOST_NS_MAGIC_CLOSING
#endif

namespace boost { namespace test_tools { BOOST_NS_MAGIC
template<>
struct print_log_value<glm::vec3> {
	void operator()( std::ostream& s , glm::vec3 const& v )
	{
		s << glm::to_string(v);
	}
};
}} BOOST_NS_MAGIC_CLOSING

namespace boost { namespace test_tools { BOOST_NS_MAGIC
template<>
struct print_log_value<std::nullptr_t> {
	void operator()( std::ostream& s , std::nullptr_t )
	{
		s << "nullptr";
	}
};
}} BOOST_NS_MAGIC_CLOSING

#undef BOOST_NS_MAGIC
#undef BOOST_NS_MAGIC_CLOSING

class Global
{
public:
	sf::Window wnd;
	GameData* d;
	GameWorld* e;
	GameState* s;
	Logger log;
	WorkContext work;
	
	Global() {
		wnd.create(sf::VideoMode(640, 360), "Testing");
		d = new GameData(&log, &work, getGamePath());

		d->loadIMG("/models/gta3");
		d->loadIMG("/anim/cuts");
		d->load();

		e = new GameWorld(&log, &work, d);
		s = new GameState;
		e->state = s;

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
