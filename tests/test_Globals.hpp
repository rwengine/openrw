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

#include <memory>

#define DATA_TEST_PREDICATE * boost::unit_test_framework::label("data-test")\
                            * boost::unit_test_framework::disabled()

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
template <>
struct print_log_value<glm::vec4> {
    void operator()(std::ostream& s, glm::vec4 const& v) {
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
    GameData* d;
    GameWorld* e;
    GameState* s;
    std::unique_ptr<GameData> d_;
    std::unique_ptr<GameWorld> e_;
    std::unique_ptr<GameState> s_;
    Logger log;

    Global() {
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
            throw std::runtime_error("Failed to initialize SDL2!");

        window.create("Tests", 800, 600, false);
        window.hideCursor();

        d_ = std::make_unique<GameData>(&log, getGamePath());
        d = d_.get();

        d->load();

        e_ = std::make_unique<GameWorld>(&log, d);
        e = e_.get();
        s_ = std::make_unique<GameState>();
        s = s_.get();
        e->state = s;

        e->dynamicsWorld->setGravity(btVector3(0.f, 0.f, 0.f));
    }

    ~Global() {
        window.close();
    }

    static std::string getGamePath();

    static Global& get() {
        static Global g;
        return g;
    }
};

#endif
