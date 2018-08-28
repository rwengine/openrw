#ifndef _TESTGLOBALS_HPP_
#define _TESTGLOBALS_HPP_

#include <boost/test/unit_test.hpp>

#include "boost_fixes.hpp"

namespace utf = boost::unit_test;
namespace tt = boost::test_tools;

#include <core/Logger.hpp>
#include <GameWindow.hpp>
#include <engine/GameData.hpp>
#include <engine/GameState.hpp>
#include <engine/GameWorld.hpp>

std::ostream& operator<<(std::ostream& stream, glm::vec3 const& v);

struct GlobalArgs {
    bool with_data = true;
};
extern GlobalArgs global_args;

class Global {
public:
    Global(const GlobalArgs& args);
    ~Global();

    static std::string getGamePath();

    GameWindow window;
    GameData* d = nullptr;
    GameWorld* e = nullptr;
    GameState* s = nullptr;
    Logger log;
    static Global& get();
};

struct with_data {
    tt::assertion_result operator()(utf::test_unit_id id) const;
};

#endif
