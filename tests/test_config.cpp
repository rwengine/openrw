#include <GameConfig.hpp>

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

#include <fstream>
#include <map>

namespace fs = boost::filesystem;

typedef std::map<std::string, std::map<std::string, std::string>> simpleConfig_t;

simpleConfig_t getValidConfig() {
    simpleConfig_t result;
    // Some values and subkeys are surrounded by whitespace
    // to test the robustness of the INI parser.
    // Don't change game.path and input.invert_y keys. Tests depend on them.
    result["game"]["path"] = "\t/dev/test  \t \r\n";
    result["game"]["\tlanguage\t "] = "      american ;american english french german italian spanish.";
    result["input"]["invert_y"] = "1 #values != 0 enable input inversion. Optional.";
    return result;
}

std::ostream &operator<<(std::ostream &os, const simpleConfig_t &config) {
    for (auto &section : config) {
        os << "[" << section.first << "]" << "\n";
        for (auto &keyValue : section.second) {
            os << keyValue.first << "=" << keyValue.second << "\n";
        }
    }
    return os;
}

class TempFile {
    // A TempFile file will be removed on destruction
public:
    TempFile() : m_path(getRandomFilePath()) {
    }
    ~TempFile() {
        this->remove();
    }
    void remove() {
        fs::remove(this->m_path);
    }
    void touch() {
        std::ofstream ofs(this->path(), std::ios::out | std::ios::app);
        ofs.close();
    }
    bool exists() {
        return fs::exists(this->m_path);
    }
    std::string path() {
        return this->m_path.string();
    }
    std::string filename() {
        return this->m_path.filename().string();
    }
    std::string dirname() {
        return this->m_path.parent_path().string();
    }
    void change_perms_readonly() {
        fs::permissions(this->m_path, fs::perms::owner_read
            | fs::perms::group_read | fs::perms::others_read);
    }
    template<typename T>
    bool append(T t) {
        // Append argument at the end of the file.
        // File is open/closes repeatedly. Not optimal.
        std::ofstream ofs(this->path(), std::ios::out | std::ios::app);
        ofs << t;
        ofs.close();
        return ofs.good();
    }
    template<typename T>
    bool write(T t) {
        // Write the argument to the file, discarding all contents.
        // File is open/closes repeatedly. Not optimal.
        std::ofstream ofs(this->path(), std::ios::out | std::ios::trunc);
        ofs << t;
        ofs.close();
        return ofs.good();
    }
private:
    static fs::path getRandomFilePath() {
        return fs::unique_path(fs::temp_directory_path() / "openrw_test_%%%%%%%%%%%%%%%%");
    }
    fs::path m_path;
};

BOOST_AUTO_TEST_SUITE(ConfigTests)

BOOST_AUTO_TEST_CASE(test_TempFile) {
    // Check the behavior of TempFile
    TempFile tempFile;
    BOOST_CHECK_EQUAL(tempFile.exists(), false);
    tempFile.touch();
    BOOST_CHECK_EQUAL(tempFile.exists(), true);
    tempFile.remove();
    BOOST_CHECK_EQUAL(tempFile.exists(), false);

    tempFile.touch();
    BOOST_CHECK_EQUAL(tempFile.exists(), true);
    tempFile.remove();

    BOOST_CHECK_EQUAL(tempFile.append("abc"), true);
    BOOST_CHECK_EQUAL(tempFile.append("def"), true);
    BOOST_CHECK_EQUAL(tempFile.exists(), true);
    tempFile.touch();
    std::ifstream ifs(tempFile.path());
    std::string line;
    std::getline(ifs, line);
    BOOST_CHECK_EQUAL(line, "abcdef");

    tempFile.change_perms_readonly();
    BOOST_CHECK_EQUAL(tempFile.write("abc"), false);
    BOOST_CHECK_EQUAL(tempFile.append("def"), false);
}

BOOST_AUTO_TEST_CASE(test_config_valid) {
    // Test reading a valid configuration file
    auto cfg = getValidConfig();

    TempFile tempFile;
    tempFile.append(cfg);

    GameConfig config(tempFile.filename(), tempFile.dirname());

    BOOST_CHECK(config.isValid());
    BOOST_CHECK_EQUAL(config.getParseResult().type(), GameConfig::ParseResult::ErrorType::GOOD);
    BOOST_CHECK_EQUAL(config.getParseResult().getKeysRequiredMissing().size(), 0);
    BOOST_CHECK_EQUAL(config.getParseResult().getKeysInvalidData().size(), 0);

    BOOST_CHECK_EQUAL(config.getGameDataPath(), "/dev/test");
    BOOST_CHECK_EQUAL(config.getGameLanguage(), "american");
    BOOST_CHECK_EQUAL(config.getInputInvertY(), true);
}

BOOST_AUTO_TEST_CASE(test_config_valid_modified) {
    // Test reading a valid modified configuration file
    auto cfg = getValidConfig();
    cfg["game"]["path"] = "Liberty City";
    cfg["input"]["invert_y"] = "0";

    TempFile tempFile;
    tempFile.append(cfg);

    GameConfig config(tempFile.filename(), tempFile.dirname());

    BOOST_CHECK(config.isValid());
    BOOST_CHECK_EQUAL(config.getParseResult().type(), GameConfig::ParseResult::ErrorType::GOOD);
    BOOST_CHECK_EQUAL(config.getParseResult().getKeysRequiredMissing().size(), 0);
    BOOST_CHECK_EQUAL(config.getParseResult().getKeysInvalidData().size(), 0);

    BOOST_CHECK_EQUAL(config.getInputInvertY(), false);
    BOOST_CHECK_EQUAL(config.getGameDataPath(), "Liberty City");
}

BOOST_AUTO_TEST_CASE(test_config_save) {
    // Test saving a configuration file
    auto cfg = getValidConfig();
    cfg["game"]["path"] = "Liberty City";

    TempFile tempFile;
    tempFile.append(cfg);

    GameConfig config(tempFile.filename(), tempFile.dirname());

    BOOST_CHECK(config.isValid());

    tempFile.remove();
    BOOST_CHECK(!tempFile.exists());

    auto writeResult = config.saveConfig();
    BOOST_CHECK(writeResult.isValid());
    BOOST_CHECK(tempFile.exists());

    GameConfig config2(tempFile.filename(), tempFile.dirname());
    BOOST_CHECK_EQUAL(config2.getGameDataPath(), "Liberty City");
}

BOOST_AUTO_TEST_CASE(test_config_save_readonly) {
    // Test whether saving to a readonly INI file fails
    auto cfg = getValidConfig();

    TempFile tempFile;
    tempFile.append(cfg);
    tempFile.change_perms_readonly();

    GameConfig config(tempFile.filename(), tempFile.dirname());
    BOOST_CHECK_EQUAL(config.isValid(), true);

    auto writeResult = config.saveConfig();
    BOOST_CHECK(!writeResult.isValid());
    BOOST_CHECK_EQUAL(writeResult.type(), GameConfig::ParseResult::ErrorType::INVALIDOUTPUTFILE);
}

BOOST_AUTO_TEST_CASE(test_config_valid_default) {
    // Test whether the default INI string is valid
    TempFile tempFile;
    BOOST_CHECK(!tempFile.exists());

    GameConfig config(tempFile.filename(), tempFile.dirname());
    BOOST_CHECK(!config.isValid());

    auto defaultINI = config.getDefaultINIString();
    tempFile.append(defaultINI);

    config = GameConfig(tempFile.filename(), tempFile.dirname());
    BOOST_CHECK(config.isValid());
}

BOOST_AUTO_TEST_CASE(test_config_invalid_duplicate) {
    // Test duplicate keys in invalid configuration file
    auto cfg = getValidConfig();
    cfg["input"]["invert_y    "] = "0";

    TempFile tempFile;
    tempFile.append(cfg);

    GameConfig config(tempFile.filename(), tempFile.dirname());

    BOOST_CHECK(!config.isValid());
    const auto &parseResult = config.getParseResult();
    BOOST_CHECK_EQUAL(parseResult.type(), GameConfig::ParseResult::ErrorType::INVALIDINPUTFILE);
}

BOOST_AUTO_TEST_CASE(test_config_invalid_required_missing) {
    // Test missing required keys in invalid configuration file
    auto cfg = getValidConfig();
    cfg["game"].erase("path");

    TempFile tempFile;
    tempFile.append(cfg);

    GameConfig config(tempFile.filename(), tempFile.dirname());

    BOOST_CHECK(!config.isValid());

    const auto &parseResult = config.getParseResult();
    BOOST_CHECK_EQUAL(parseResult.type(), GameConfig::ParseResult::ErrorType::INVALIDCONTENT);

    BOOST_CHECK_EQUAL(parseResult.getKeysRequiredMissing().size(), 1);
    BOOST_CHECK_EQUAL(parseResult.getKeysInvalidData().size(), 0);

    BOOST_CHECK_EQUAL(parseResult.getKeysRequiredMissing()[0], "game.path");
}

BOOST_AUTO_TEST_CASE(test_config_invalid_wrong_type) {
    // Test wrong data type
    auto cfg = getValidConfig();
    cfg["input"]["invert_y"]="d";

    TempFile tempFile;
    tempFile.append(cfg);

    GameConfig config(tempFile.filename(), tempFile.dirname());

    BOOST_CHECK(!config.isValid());

    const auto &parseResult = config.getParseResult();
    BOOST_CHECK_EQUAL(parseResult.type(), GameConfig::ParseResult::ErrorType::INVALIDCONTENT);

    BOOST_CHECK_EQUAL(parseResult.getKeysRequiredMissing().size(), 0);
    BOOST_CHECK_EQUAL(parseResult.getKeysInvalidData().size(), 1);

    BOOST_CHECK_EQUAL(parseResult.getKeysInvalidData()[0], "input.invert_y");
}

BOOST_AUTO_TEST_CASE(test_config_invalid_empty) {
    // Test reading empty configuration file
    // An empty file has a valid data structure, but has missing keys and is thus invalid.
    TempFile tempFile;
    tempFile.touch();
    BOOST_CHECK(tempFile.exists());

    GameConfig config(tempFile.filename(), tempFile.dirname());

    BOOST_CHECK(!config.isValid());

    const auto &parseResult = config.getParseResult();
    BOOST_CHECK_EQUAL(parseResult.type(), GameConfig::ParseResult::ErrorType::INVALIDCONTENT);
    BOOST_CHECK_GE(parseResult.getKeysRequiredMissing().size(), 1);
}

BOOST_AUTO_TEST_CASE(test_config_invalid_nonexisting) {
    // Test reading non-existing configuration file
    TempFile tempFile;

    BOOST_CHECK(!tempFile.exists());
    GameConfig config(tempFile.filename(), tempFile.dirname());

    BOOST_CHECK(!config.isValid());

    const auto &parseResult = config.getParseResult();
    BOOST_CHECK_EQUAL(parseResult.type(), GameConfig::ParseResult::ErrorType::INVALIDINPUTFILE);
}

BOOST_AUTO_TEST_SUITE_END()
