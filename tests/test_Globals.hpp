#ifndef _TESTGLOBALS_HPP_
#define _TESTGLOBALS_HPP_

#ifdef _MSC_VER
#pragma warning(disable : 4305)
#endif
#include <btBulletDynamicsCommon.h>
#ifdef _MSC_VER
#pragma warning(default : 4305)
#endif

#include <SDL.h>
#include <GameWindow.hpp>
#include <boost/test/unit_test.hpp>
#include <core/Logger.hpp>
#include <engine/GameData.hpp>
#include <engine/GameState.hpp>
#include <engine/GameWorld.hpp>
#include <objects/GameObject.hpp>
#include <glm/gtx/string_cast.hpp>

std::ostream& operator<<(std::ostream& stream, glm::vec3 const& v);

// Boost moved the print_log_value struct in version 1.59
// TODO: use another testing library
#if BOOST_VERSION >= 105900
#define BOOST_NS_MAGIC namespace tt_detail {
#define BOOST_NS_MAGIC_CLOSING }
#else
#define BOOST_NS_MAGIC
#define BOOST_NS_MAGIC_CLOSING
#endif

namespace boost {
namespace test_tools {
BOOST_NS_MAGIC
template <>
struct print_log_value<glm::vec3> {
    void operator()(std::ostream& s, glm::vec3 const& v) {
        s << glm::to_string(v);
    }
};
BOOST_NS_MAGIC_CLOSING
}
}

#if BOOST_VERSION < 106400
namespace boost {
namespace test_tools {
BOOST_NS_MAGIC
template <>
struct print_log_value<std::nullptr_t> {
    void operator()(std::ostream& s, std::nullptr_t) {
        s << "nullptr";
    }
};
BOOST_NS_MAGIC_CLOSING
}
}
#endif

namespace boost {
namespace test_tools {
BOOST_NS_MAGIC
template <>
struct print_log_value<GameString> {
    void operator()(std::ostream& s, GameString const& v) {
        for (GameString::size_type i = 0u; i < v.size(); ++i) {
            s << static_cast<char>(v[i]);
        }
    }
};
BOOST_NS_MAGIC_CLOSING
}
}

#undef BOOST_NS_MAGIC
#undef BOOST_NS_MAGIC_CLOSING

class Global {
public:
    GameWindow window;
#if RW_TEST_WITH_DATA
    GameData* d;
    GameWorld* e;
    GameState* s;
    Logger log;
#endif

    Global() {
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
            throw std::runtime_error("Failed to initialize SDL2!");

        window.create("Tests", 800, 600, false);
        window.hideCursor();

#if RW_TEST_WITH_DATA
        d = new GameData(&log, getGamePath());

        d->load();

        e = new GameWorld(&log, d);
        s = new GameState;
        e->state = s;

        e->dynamicsWorld->setGravity(btVector3(0.f, 0.f, 0.f));
#endif
    }

    ~Global() {
        window.close();
#if RW_TEST_WITH_DATA
        delete e;
#endif
    }

#if RW_TEST_WITH_DATA
    static std::string getGamePath();
#endif

    static Global& get() {
        static Global g;
        return g;
    }
};

#endif
