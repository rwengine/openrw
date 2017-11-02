#ifndef RWGAME_GAMEBASE_HPP
#define RWGAME_GAMEBASE_HPP
#include "GameConfig.hpp"
#include "GameWindow.hpp"

#include <core/Logger.hpp>

#include <boost/program_options.hpp>

/**
 * @brief Handles basic window and setup
 */
class GameBase {
public:
    GameBase(Logger& inlog, int argc, char* argv[]);
    ~GameBase();

    GameWindow& getWindow() {
        return window;
    }

    const GameConfig& getConfig() const {
        return config;
    }

protected:
    Logger& log;
    GameConfig config;
    GameWindow window;
    boost::program_options::variables_map options;
};

#endif
