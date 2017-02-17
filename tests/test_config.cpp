#include <GameConfig.hpp>

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

#include <fstream>
#include <map>

namespace fs = boost::filesystem;

typedef std::map<std::string, std::map<std::string, std::string>> simpleConfig_t;
     
fs::path getRandomFilePath() {
    return fs::unique_path(fs::temp_directory_path() /= "openrw_test_%%%%%%%%%%%%%%%%");
}

simpleConfig_t getValidConfig() {
    simpleConfig_t result;
    // Don't change game.path and input.invert_y keys. Tests depend on them.
    result["game"]["path"] = "\t/dev/test  \t \r\n";
    result["game"]["\tlanguage\t "] = "      american ;american english french german italian spanish.";
    result["input"]["invert_y"] = "1 #values != 0 enable input inversion. Optional.";
    return result;
}

std::ostream &writeConfig(std::ostream &os, const simpleConfig_t &config) {
    for (auto &section : config) {
        os << "[" << section.first << "]" << "\n";
        for (auto &keyValue : section.second) {
            os << keyValue.first << "=" << keyValue.second << "\n";
        }
    }
    return os;
}

BOOST_AUTO_TEST_SUITE(ConfigTests)

BOOST_AUTO_TEST_CASE(test_config_valid) {
    // Test reading a valid configuration file
    auto cfg = getValidConfig();
    auto configPath = getRandomFilePath();

    std::ofstream ofs(configPath.string());
    writeConfig(ofs, cfg);
    ofs.close();

    GameConfig config(configPath.filename().string(),
            configPath.parent_path().string());

    BOOST_CHECK(config.isValid());

    BOOST_CHECK_EQUAL(config.getGameDataPath(), "/dev/test");
    BOOST_CHECK_EQUAL(config.getGameLanguage(), "american");
    BOOST_CHECK_EQUAL(config.getInputInvertY(), true);
}

BOOST_AUTO_TEST_CASE(test_config_valid_modified) {
    // Test reading a valid modified configuration file
    auto cfg = getValidConfig();
    cfg["game"]["path"] = "Liberty City";
    cfg["input"]["invert_y"] = "0";
    auto configPath = getRandomFilePath();

    std::ofstream ofs(configPath.string());
    writeConfig(ofs, cfg);
    ofs.close();

    GameConfig config(configPath.filename().string(),
            configPath.parent_path().string());

    BOOST_CHECK(config.isValid());

    BOOST_CHECK_EQUAL(config.getInputInvertY(), false);
    BOOST_CHECK_EQUAL(config.getGameDataPath(), "Liberty City");
}

BOOST_AUTO_TEST_CASE(test_config_save) {
    // Test saving a configuration file
    auto cfg = getValidConfig();
    cfg["game"]["path"] = "Liberty City";
    auto configPath = getRandomFilePath();

    std::ofstream ofs(configPath.string());
    writeConfig(ofs, cfg);
    ofs.close();

    GameConfig config(configPath.filename().string(),
            configPath.parent_path().string());

    BOOST_CHECK(config.isValid());
    
    fs::remove(configPath);
    
    BOOST_CHECK(!fs::exists(configPath));
    BOOST_CHECK(config.saveConfig());
    BOOST_CHECK(fs::exists(configPath));
    
    GameConfig config2(configPath.filename().string(),
            configPath.parent_path().string());
    
    BOOST_CHECK_EQUAL(config2.getGameDataPath(), "Liberty City");
}

BOOST_AUTO_TEST_CASE(test_config_invalid_duplicate) {
    // Test duplicate keys in invalid configuration file
    auto cfg = getValidConfig();
    cfg["input"]["invert_y    "] = "0";
    auto configPath = getRandomFilePath();

    std::ofstream ofs(configPath.string());
    writeConfig(ofs, cfg);
    ofs.close();

    GameConfig config(configPath.filename().string(),
            configPath.parent_path().string());

    BOOST_CHECK(!config.isValid());
}

BOOST_AUTO_TEST_CASE(test_config_invalid_required_missing) {
    // Test missing required keys in invalid configuration file
    auto cfg = getValidConfig();
    cfg["game"].erase("path");
    auto configPath = getRandomFilePath();

    std::ofstream ofs(configPath.string());
    writeConfig(ofs, cfg);
    ofs.close();

    GameConfig config(configPath.filename().string(),
            configPath.parent_path().string());

    BOOST_CHECK(!config.isValid());
}

BOOST_AUTO_TEST_CASE(test_config_invalid_empty) {
    // Test reading empty configuration file
    simpleConfig_t cfg;
    auto configPath = getRandomFilePath();

    std::ofstream ofs(configPath.string());
    writeConfig(ofs, cfg);
    ofs.close();

    GameConfig config(configPath.filename().string(),
            configPath.parent_path().string());

    BOOST_CHECK(!config.isValid());
}

BOOST_AUTO_TEST_CASE(test_config_invalid_nonexisting) {
    // Test reading non-existing configuration file
    auto configPath = getRandomFilePath();

    GameConfig config(configPath.filename().string(),
            configPath.parent_path().string());

    BOOST_CHECK(!config.isValid());
}

BOOST_AUTO_TEST_SUITE_END()
