#include "test_Globals.hpp"

#include <GameConfig.hpp>

#if RW_TEST_WITH_DATA
std::string Global::getGamePath() {
    GameConfig config;
    config.loadFile(GameConfig::getDefaultConfigPath() / "openrw.ini");
    return config.getGameDataPath();
}
#endif
