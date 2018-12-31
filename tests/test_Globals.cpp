#include "test_Globals.hpp"

#include <RWConfig.hpp>

#if RW_TEST_WITH_DATA
std::string Global::getGamePath() {
    rwfs::path configPath = RWConfigParser::getDefaultConfigPath() / "openrw.ini";
    RWConfigParser cfgParser;
    auto [cfgLayer, parseResult] = cfgParser.loadFile(configPath);
    BOOST_REQUIRE(parseResult.isValid());
    return *cfgLayer.gamedataPath;
}
#endif
