#include <GameConfig.hpp>

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

#include <fstream>
#include <map>

namespace fs = boost::filesystem;

typedef std::map<std::string, std::map<std::string, std::string>> config_t;
     
fs::path getRandomFilePath() {
    return fs::unique_path(fs::temp_directory_path() /= "openrw_test_%%%%%%%%%%%%%%%%");
}

config_t getValidConfig() {
    config_t result;
    result["game"]["\tpath    "] = "\t/dev/test  \t \r\n";
    result["game"]["language\t "] = "      american ;american english french german italian spanish.";
    result["input"]["invert_y"] = "1 #values != 0 enable input inversion. Optional.";
    return result;
}

std::ostream &writeConfig(std::ostream &os, const config_t &config) {
    for (auto &section : config) {
        os << "[" << section.first << "]" << "\n";
        for (auto &keyValue : section.second) {
            os << keyValue.first << "=" << keyValue.second << "\n";
        }
    }
    return os;
}

#include <iostream>

BOOST_AUTO_TEST_SUITE(ConfigTests)

BOOST_AUTO_TEST_CASE(test_config_valid) {
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
    auto cfg = getValidConfig();
    cfg["input"]["invert_y"] = "0";
    auto configPath = getRandomFilePath();

    std::ofstream ofs(configPath.string());
    writeConfig(ofs, cfg);
    ofs.close();

    GameConfig config(configPath.filename().string(),
            configPath.parent_path().string());

    BOOST_CHECK(config.isValid());

    BOOST_CHECK_EQUAL(config.getInputInvertY(), false);
}

BOOST_AUTO_TEST_CASE(test_config_invalid_duplicate) {
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

BOOST_AUTO_TEST_CASE(test_config_invalid_empty) {
    config_t cfg;
    auto configPath = getRandomFilePath();
    std::cout << configPath.string() << "\n";

    std::ofstream ofs(configPath.string());
    writeConfig(ofs, cfg);
    ofs.close();

    GameConfig config(configPath.filename().string(),
            configPath.parent_path().string());

    BOOST_CHECK(!config.isValid());
}

BOOST_AUTO_TEST_SUITE_END()
