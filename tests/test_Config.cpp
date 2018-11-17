#include <GameConfig.hpp>

#include <boost/test/unit_test.hpp>

#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <fstream>
#include <map>

#include <rw/debug.hpp>
#include <rw/filesystem.hpp>

namespace pt = boost::property_tree;

typedef std::map<std::string, std::map<std::string, std::string>>
    simpleConfig_t;

simpleConfig_t readConfig(const rwfs::path& path) {
    simpleConfig_t cfg;
    pt::ptree tree;
    pt::read_ini(path.string(), tree);

    for (const auto& section : tree) {
        for (const auto& subKey : section.second) {
            cfg[section.first][subKey.first] = subKey.second.data();
        }
    }
    return cfg;
}

std::string stripWhitespace(const std::string& in) {
    static const std::string whitespace = " \n\r\t";
    auto start = in.find_first_not_of(whitespace);
    auto end = in.find_last_not_of(whitespace) + 1;
    return std::string(in, start, end - start);
}

simpleConfig_t getValidConfig() {
    simpleConfig_t result;
    // Some values and subkeys are surrounded by whitespace
    // to test the robustness of the INI parser.
    // Don't change game.path and input.invert_y keys. Tests depend on them.
    result["game"]["path"] = "\t/dev/test  \t \r\n";
    result["game"]["\tlanguage\t "] =
        "      american ;american english french german italian spanish.";
    result["input"]["invert_y"] =
        "1 #values != 0 enable input inversion. Optional.";
    result["game"]["hud_scale"] = "2.0\t;HUD scale";
    return result;
}

std::ostream& operator<<(std::ostream& os, const simpleConfig_t& config) {
    for (auto& section : config) {
        os << "[" << section.first << "]"
           << "\n";
        for (auto& keyValue : section.second) {
            os << keyValue.first << "=" << keyValue.second << "\n";
        }
    }
    return os;
}

class Temp {
    // An object of type Temp file will be removed on destruction
public:
    virtual ~Temp() {
    }
    bool exists() const {
        return rwfs::exists(this->m_path);
    }
    const rwfs::path& path() const {
        return this->m_path;
    }
    std::string filename() const {
        return this->m_path.filename().string();
    }
    std::string dirname() const {
        return this->m_path.parent_path().string();
    }
    virtual void change_perms_normal() const = 0;
    virtual void change_perms_readonly() const = 0;
    virtual void remove() const = 0;
    virtual void touch() const = 0;

protected:
    Temp(const Temp&) = delete;
    Temp() : m_path(getRandomFilePath()) {
    }
    Temp(const rwfs::path& dirname) : m_path(getRandomFilePath(dirname)) {
    }

private:
    static rwfs::path getRandomFilePath(const rwfs::path& dirname) {
        return rwfs::unique_path(dirname / "openrw_test_%%%%%%%%%%%%%%%%");
    }
    static rwfs::path getRandomFilePath() {
        return getRandomFilePath(rwfs::temp_directory_path());
    }
    rwfs::path m_path;
};

class TempFile;

class TempDir : public Temp {
public:
    TempDir() : Temp() {
    }
    TempDir(const TempDir& dirname) : Temp(dirname.path()) {
    }
    virtual ~TempDir() {
        this->remove();
    }
    virtual void change_perms_normal() const override {
        rwfs::permissions(this->path(),
                          rwfs::perms::owner_read | rwfs::perms::owner_write |
                              rwfs::perms::owner_exe | rwfs::perms::group_read |
                              rwfs::perms::group_exe |
                              rwfs::perms::others_read |
                              rwfs::perms::others_exe);
    }
    virtual void change_perms_readonly() const override {
        rwfs::permissions(this->path(),
                          rwfs::perms::owner_read | rwfs::perms::owner_exe |
                              rwfs::perms::group_read | rwfs::perms::group_exe |
                              rwfs::perms::others_read |
                              rwfs::perms::others_exe);
    }
    virtual void remove() const override {
        // Remove may fail if this directory contains a read-only entry. Ignore.
        rwfs::error_code ec;
        rwfs::remove_all(this->path(), ec);
    }
    void touch() const override {
        rwfs::create_directories(this->path());
    }
    friend class TempFile;
};

class TempFile : public Temp {
public:
    TempFile() : Temp() {
    }
    TempFile(const TempDir& dirname) : Temp(dirname.path()) {
    }
    virtual ~TempFile() {
        this->remove();
    }
    virtual void change_perms_normal() const override {
        rwfs::permissions(this->path(), rwfs::perms::owner_read |
                                            rwfs::perms::owner_write |
                                            rwfs::perms::group_read |
                                            rwfs::perms::others_read);
    }
    virtual void change_perms_readonly() const override {
        rwfs::permissions(this->path(), rwfs::perms::owner_read |
                                            rwfs::perms::group_read |
                                            rwfs::perms::others_read);
    }
    virtual void remove() const override {
        rwfs::error_code ec;
        rwfs::remove_all(this->path(), ec);
    }
    virtual void touch() const override {
        std::ofstream ofs(this->path().string(), std::ios::out | std::ios::app);
        ofs.close();
    }
    template<typename T>
    bool append(T t) const {
        // Append argument at the end of the file.
        // File is open/closes repeatedly. Not optimal.
        std::ofstream ofs(this->path().string(), std::ios::out | std::ios::app);
        ofs << t;
        ofs.close();
        return ofs.good();
    }
    template<typename T>
    bool write(T t) const {
        // Write the argument to the file, discarding all contents.
        // File is open/closes repeatedly. Not optimal.
        std::ofstream ofs(this->path().string(),
                          std::ios::out | std::ios::trunc);
        ofs << t;
        ofs.close();
        return ofs.good();
    }
};

BOOST_AUTO_TEST_SUITE(ConfigTests)

BOOST_AUTO_TEST_CASE(test_stripWhitespace) {
    std::map<std::string, std::string> map;
    map["abc"] = "abc";
    map["\tabc"] = "abc";
    map["abc\t"] = "abc";
    map[" abc"] = "abc";
    map["abc "] = "abc";
    map[" abc "] = "abc";
    map["  abc  "] = "abc";
    for (const auto& keyValue : map) {
        BOOST_CHECK_EQUAL(keyValue.second, stripWhitespace(keyValue.first));
    }
}

BOOST_AUTO_TEST_CASE(test_TempDir) {
    // Check the behavior of TempFile
    TempDir tempDir;
    BOOST_CHECK(!tempDir.exists());
    tempDir.touch();
    BOOST_CHECK(tempDir.exists());
    tempDir.remove();
    BOOST_CHECK(!tempDir.exists());

    tempDir.touch();
    BOOST_CHECK(tempDir.exists());

    TempDir tempChildDir(tempDir);
    BOOST_CHECK(!tempChildDir.exists());

    tempChildDir.touch();
    BOOST_CHECK(tempChildDir.exists());

    tempDir.remove();
    BOOST_CHECK(!tempChildDir.exists());
    BOOST_CHECK(!tempDir.exists());

    tempChildDir.touch();
    BOOST_CHECK(tempChildDir.exists());

    rwfs::path path;
    {
        TempDir tempLocal;
        tempLocal.touch();
        BOOST_CHECK(tempLocal.exists());
        path = tempLocal.path();
    }
    BOOST_CHECK(!rwfs::exists(path));
}

BOOST_AUTO_TEST_CASE(test_TempFile) {
    // Check the behavior of TempFile
    TempFile tempFile;
    BOOST_CHECK(!tempFile.exists());
    tempFile.touch();
    BOOST_CHECK(tempFile.exists());
    tempFile.remove();
    BOOST_CHECK(!tempFile.exists());

    tempFile.touch();
    BOOST_CHECK(tempFile.exists());
    tempFile.remove();

    BOOST_CHECK(tempFile.append("abc"));
    BOOST_CHECK(tempFile.append("def"));
    BOOST_CHECK(tempFile.exists());
    tempFile.touch();
    std::ifstream ifs(tempFile.path().string());
    std::string line;
    std::getline(ifs, line);
    BOOST_CHECK_EQUAL(line, "abcdef");

    tempFile.change_perms_readonly();
    BOOST_CHECK(!tempFile.write("abc"));
    BOOST_CHECK(!tempFile.append("def"));

    rwfs::path path;
    {
        TempFile tempLocal;
        tempLocal.touch();
        BOOST_CHECK(tempLocal.exists());
        path = tempLocal.path();
    }
    BOOST_CHECK(!rwfs::exists(path));
}

BOOST_AUTO_TEST_CASE(test_config_initial) {
    // Test an initial config
    GameConfig cfg;
    BOOST_CHECK(!cfg.isValid());
}

BOOST_AUTO_TEST_CASE(test_config_valid) {
    // Test reading a valid configuration file
    auto cfg = getValidConfig();

    TempFile tempFile;
    tempFile.append(cfg);

    GameConfig config;
    config.loadFile(tempFile.path());

    BOOST_CHECK(config.isValid());
    BOOST_CHECK_EQUAL(config.getParseResult().type(),
                      GameConfig::ParseResult::ErrorType::GOOD);
    BOOST_CHECK_EQUAL(config.getParseResult().getKeysRequiredMissing().size(),
                      0);
    BOOST_CHECK_EQUAL(config.getParseResult().getKeysInvalidData().size(), 0);

    BOOST_CHECK_EQUAL(config.getGameDataPath().string(), "/dev/test");
    BOOST_CHECK_EQUAL(config.getGameLanguage(), "american");
    BOOST_CHECK(config.getInputInvertY());
    BOOST_CHECK_EQUAL(config.getHUDScale(), 2.f);
}

BOOST_AUTO_TEST_CASE(test_config_valid_modified) {
    // Test reading a valid modified configuration file
    auto cfg = getValidConfig();
    cfg["game"]["path"] = "Liberty City";
    cfg["input"]["invert_y"] = "0";

    TempFile tempFile;
    tempFile.append(cfg);

    GameConfig config;
    config.loadFile(tempFile.path());

    BOOST_CHECK(config.isValid());
    BOOST_CHECK_EQUAL(config.getParseResult().type(),
                      GameConfig::ParseResult::ErrorType::GOOD);
    BOOST_CHECK_EQUAL(config.getParseResult().getKeysRequiredMissing().size(),
                      0);
    BOOST_CHECK_EQUAL(config.getParseResult().getKeysInvalidData().size(), 0);

    BOOST_CHECK(!config.getInputInvertY());
    BOOST_CHECK_EQUAL(config.getGameDataPath().string(), "Liberty City");
}

BOOST_AUTO_TEST_CASE(test_config_save) {
    // Test saving a configuration file
    auto cfg = getValidConfig();
    cfg["game"]["path"] = "Liberty City";

    TempFile tempFile;
    tempFile.append(cfg);

    GameConfig config;
    config.loadFile(tempFile.path());

    BOOST_CHECK(config.isValid());

    tempFile.remove();
    BOOST_CHECK(!tempFile.exists());

    auto writeResult = config.saveConfig();
    BOOST_CHECK(writeResult.isValid());
    BOOST_CHECK(tempFile.exists());

    GameConfig config2;
    config2.loadFile(tempFile.path());
    BOOST_CHECK_EQUAL(config2.getGameDataPath().string(), "Liberty City");

    simpleConfig_t cfg2 = readConfig(tempFile.path());
    BOOST_CHECK_EQUAL(cfg2["game"]["path"], "Liberty City");
}

BOOST_AUTO_TEST_CASE(test_config_valid_unknown_keys) {
    // Test reading a valid modified configuration file with unknown data
    auto cfg = getValidConfig();
    cfg["game"]["unknownkey"] = "descartes";
    cfg["dontknow"]["dontcare"] = "\t$%!$8847  %%$  ";

    TempFile tempFile;
    tempFile.append(cfg);

    GameConfig config;
    config.loadFile(tempFile.path());

    BOOST_CHECK(config.isValid());

    const auto& unknownData = config.getParseResult().getUnknownData();

    BOOST_CHECK_EQUAL(unknownData.size(), 2);

    BOOST_CHECK_EQUAL(unknownData.count("game.unknownkey"), 1);
    BOOST_CHECK_EQUAL(unknownData.at("game.unknownkey"),
                      stripWhitespace(cfg["game"]["unknownkey"]));

    BOOST_CHECK_EQUAL(unknownData.count("dontknow.dontcare"), 1);
    BOOST_CHECK_EQUAL(unknownData.at("dontknow.dontcare"),
                      stripWhitespace(cfg["dontknow"]["dontcare"]));

    BOOST_CHECK_EQUAL(unknownData.count("game.path"), 0);

    tempFile.remove();
    config.saveConfig();

    GameConfig config2;
    config2.loadFile(tempFile.path());
    const auto& unknownData2 = config2.getParseResult().getUnknownData();

    BOOST_CHECK_EQUAL(unknownData2.size(), 2);

    BOOST_CHECK_EQUAL(unknownData2.count("game.unknownkey"), 1);
    BOOST_CHECK_EQUAL(unknownData2.at("game.unknownkey"),
                      stripWhitespace(cfg["game"]["unknownkey"]));

    BOOST_CHECK_EQUAL(unknownData2.count("dontknow.dontcare"), 1);
    BOOST_CHECK_EQUAL(unknownData2.at("dontknow.dontcare"),
                      stripWhitespace(cfg["dontknow"]["dontcare"]));

    BOOST_CHECK_EQUAL(unknownData2.count("game.path"), 0);
}

BOOST_AUTO_TEST_CASE(test_config_save_readonly) {
    // Test whether saving to a readonly INI file fails
    auto cfg = getValidConfig();

    TempFile tempFile;
    tempFile.append(cfg);
    tempFile.change_perms_readonly();

    GameConfig config;
    config.loadFile(tempFile.path());
    BOOST_CHECK(config.isValid());

    auto writeResult = config.saveConfig();
    BOOST_CHECK(!writeResult.isValid());
    BOOST_CHECK_EQUAL(writeResult.type(),
                      GameConfig::ParseResult::ErrorType::INVALIDOUTPUTFILE);
}

BOOST_AUTO_TEST_CASE(test_config_valid_default) {
    // Test whether the default INI string is valid
    TempFile tempFile;
    BOOST_CHECK(!tempFile.exists());

    GameConfig config;
    config.loadFile(tempFile.path());
    BOOST_CHECK(!config.isValid());

    auto defaultINI = config.getDefaultINIString();
    tempFile.append(defaultINI);
    BOOST_CHECK(tempFile.exists());

    config.loadFile(tempFile.path());
    BOOST_CHECK(config.isValid());
}

BOOST_AUTO_TEST_CASE(test_config_invalid_emptykey) {
    // Test duplicate keys in invalid configuration file
    auto cfg = getValidConfig();
    cfg["game"][""] = "0";

    TempFile tempFile;
    tempFile.append(cfg);

    GameConfig config;
    config.loadFile(tempFile.path());

    BOOST_CHECK(!config.isValid());
    const auto& parseResult = config.getParseResult();
    BOOST_CHECK_EQUAL(parseResult.type(),
                      GameConfig::ParseResult::ErrorType::INVALIDINPUTFILE);
}

BOOST_AUTO_TEST_CASE(test_config_invalid_duplicate) {
    // Test duplicate keys in invalid configuration file
    auto cfg = getValidConfig();
    cfg["input"]["invert_y    "] = "0";

    TempFile tempFile;
    tempFile.append(cfg);

    GameConfig config;
    config.loadFile(tempFile.path());

    BOOST_CHECK(!config.isValid());
    const auto& parseResult = config.getParseResult();
    BOOST_CHECK_EQUAL(parseResult.type(),
                      GameConfig::ParseResult::ErrorType::INVALIDINPUTFILE);
}

BOOST_AUTO_TEST_CASE(test_config_invalid_required_missing) {
    // Test missing required keys in invalid configuration file
    auto cfg = getValidConfig();
    cfg["game"].erase("path");

    TempFile tempFile;
    tempFile.append(cfg);

    GameConfig config;
    config.loadFile(tempFile.path());

    BOOST_CHECK(!config.isValid());

    const auto& parseResult = config.getParseResult();
    BOOST_CHECK_EQUAL(parseResult.type(),
                      GameConfig::ParseResult::ErrorType::INVALIDCONTENT);

    BOOST_CHECK_EQUAL(parseResult.getKeysRequiredMissing().size(), 1);
    BOOST_CHECK_EQUAL(parseResult.getKeysInvalidData().size(), 0);

    BOOST_CHECK_EQUAL(parseResult.getKeysRequiredMissing()[0], "game.path");
}

BOOST_AUTO_TEST_CASE(test_config_invalid_wrong_type) {
    // Test wrong data type
    auto cfg = getValidConfig();
    cfg["input"]["invert_y"] = "d";

    TempFile tempFile;
    tempFile.append(cfg);

    GameConfig config;
    config.loadFile(tempFile.path());

    BOOST_CHECK(!config.isValid());

    const auto& parseResult = config.getParseResult();
    BOOST_CHECK_EQUAL(parseResult.type(),
                      GameConfig::ParseResult::ErrorType::INVALIDCONTENT);

    BOOST_CHECK_EQUAL(parseResult.getKeysRequiredMissing().size(), 0);
    BOOST_CHECK_EQUAL(parseResult.getKeysInvalidData().size(), 1);

    BOOST_CHECK_EQUAL(parseResult.getKeysInvalidData()[0], "input.invert_y");
}

BOOST_AUTO_TEST_CASE(test_config_invalid_empty) {
    // Test reading empty configuration file
    // An empty file has a valid data structure, but has missing keys and is
    // thus invalid.
    TempFile tempFile;
    tempFile.touch();
    BOOST_CHECK(tempFile.exists());

    GameConfig config;
    config.loadFile(tempFile.path());

    BOOST_CHECK(!config.isValid());

    const auto& parseResult = config.getParseResult();
    BOOST_CHECK_EQUAL(parseResult.type(),
                      GameConfig::ParseResult::ErrorType::INVALIDCONTENT);
    BOOST_CHECK_GE(parseResult.getKeysRequiredMissing().size(), 1);
}

BOOST_AUTO_TEST_CASE(test_config_invalid_nodir) {
    // Test reading non-existing configuration file in non-existing directory
    TempDir tempDir;
    TempFile tempFile(tempDir);

    BOOST_CHECK(!tempDir.exists());
    BOOST_CHECK(!tempFile.exists());

    GameConfig config;
    config.loadFile(tempFile.path());

    BOOST_CHECK(!config.isValid());

    const auto& parseResult = config.getParseResult();
    BOOST_CHECK_EQUAL(parseResult.type(),
                      GameConfig::ParseResult::ErrorType::INVALIDINPUTFILE);
}

BOOST_AUTO_TEST_CASE(test_config_invalid_nonexisting) {
    // Test reading non-existing configuration file
    TempFile tempFile;

    BOOST_CHECK(!tempFile.exists());
    GameConfig config;
    config.loadFile(tempFile.path());

    BOOST_CHECK(!config.isValid());

    const auto& parseResult = config.getParseResult();
    BOOST_CHECK_EQUAL(parseResult.type(),
                      GameConfig::ParseResult::ErrorType::INVALIDINPUTFILE);
}

BOOST_AUTO_TEST_SUITE_END()
