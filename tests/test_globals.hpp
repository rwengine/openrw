#ifndef _TESTGLOBABLS_HPP_
#define _TESTGLOBABLS_HPP_

#include <engine/GameWorld.hpp>
#include <engine/GameData.hpp>
#include <engine/GameState.hpp>
#include <core/Logger.hpp>
#include <glm/gtx/string_cast.hpp>
#include <SDL2/SDL.h>
#include <GameWindow.hpp>
#include <GameConfig.hpp>

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

namespace boost { namespace test_tools { BOOST_NS_MAGIC
template<>
struct print_log_value<GameString> {
	void operator()( std::ostream& s , GameString const& v )
	{
		for (GameString::size_type i = 0u; i<v.size(); ++i) {
			s << (char)v[i];
		}
	}
};
}} BOOST_NS_MAGIC_CLOSING

#undef BOOST_NS_MAGIC
#undef BOOST_NS_MAGIC_CLOSING

class Global
{
public:
	GameWindow window;
#if RW_TEST_WITH_DATA
	GameData* d;
	GameWorld* e;
	GameState* s;
	Logger log;
	WorkContext work;
#endif

	Global() {
		if (SDL_Init(SDL_INIT_VIDEO) < 0)
			throw std::runtime_error("Failed to initialize SDL2!");

		window.create("Tests", 800, 600, false);
		window.hideCursor();

#if RW_TEST_WITH_DATA
		d = new GameData(&log, &work, getGamePath());

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
#endif
	}

	~Global() {
		window.close();
#if RW_TEST_WITH_DATA
		delete e;
#endif
	}

#if RW_TEST_WITH_DATA
	static std::string getGamePath()
	{
		return GameConfig("openrw.ini").getGameDataPath();
	}
#endif

	static Global& get()
	{
		static Global g;
		return g;
	}
};

#endif
