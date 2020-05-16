#include <RWConfig.hpp>

#include <boost/test/unit_test.hpp>

#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <map>
#include <random>
#include <string>
#include <string_view>

#include <rw/debug.hpp>
#include <rw/filesystem.hpp>

#if RW_FS_LIBRARY == RW_FS_BOOST
//
// boost misnamed some permissions from filesystem::perms badly.
// Fix that for now.
//
constexpr rwfs::perms kNormalFilePermissions{
    rwfs::perms::owner_all |
    rwfs::perms::group_read | rwfs::perms::group_exe |
    rwfs::perms::others_read | rwfs::perms::others_exe
};

constexpr rwfs::perms kReadonlyFilePermissions{
    rwfs::perms::owner_read | rwfs::perms::owner_exe |
    rwfs::perms::group_read | rwfs::perms::group_exe |
    rwfs::perms::others_read | rwfs::perms::others_exe
};
#else
constexpr rwfs::perms kNormalFilePermissions{
    rwfs::perms::owner_all |
    rwfs::perms::group_read | rwfs::perms::group_exec |
    rwfs::perms::others_read | rwfs::perms::others_exec
};

constexpr rwfs::perms kReadonlyFilePermissions{
    rwfs::perms::owner_read | rwfs::perms::owner_exec |
    rwfs::perms::group_read | rwfs::perms::group_exec |
    rwfs::perms::others_read | rwfs::perms::others_exec
};
#endif

namespace pt = boost::property_tree;

typedef std::map<std::string, std::map<std::string, std::string>>
    simpleConfig_t;

namespace {

simpleConfig_t readConfig(const rwfs::path &path) {
    simpleConfig_t cfg;
    pt::ptree tree;
    pt::read_ini(path.string(), tree);

    for (const auto &section : tree) {
        for (const auto &subKey : section.second) {
            cfg[section.first][subKey.first] = subKey.second.data();
        }
    }
    return cfg;
}

std::string stripWhitespace(const std::string &in) {
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

std::string generateRandomString(std::size_t length)
{
    static constexpr std::string_view kCharacters{
        "0123456789"
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    };

    static std::mt19937 generator{std::random_device{}()};
    static std::uniform_int_distribution<std::string::size_type> dist(0, kCharacters.size() - 1);

    std::string result(length, '\0');
    std::generate(
        result.begin(), result.end(),
        [&]() { return kCharacters[dist(generator)]; }
    );

    return result;
}

} // namespace

std::ostream &operator<<(std::ostream &os, const simpleConfig_t &config) {
    for (auto &section : config) {
        os << "[" << section.first << "]"
           << "\n";
        for (auto &keyValue : section.second) {
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
    const rwfs::path &path() const {
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
    Temp(const Temp &) = delete;
    Temp() : m_path(getRandomFilePath()) {
    }
    Temp(const rwfs::path &dirname) : m_path(getRandomFilePath(dirname)) {
    }

private:
    static rwfs::path getRandomFilePath(const rwfs::path &dirname) {
        //
        // If std::filesystem gains an equivalent to unique_path, use that here
        // instead of generating our own random name.
        //

        std::string filename = "openrw_test_" + generateRandomString(16);
        return dirname / filename;
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
    TempDir(const TempDir &dirname) : Temp(dirname.path()) {
    }
    virtual ~TempDir() {
        this->remove();
    }
    virtual void change_perms_normal() const override {
        rwfs::permissions(this->path(), kNormalFilePermissions);
    }
    virtual void change_perms_readonly() const override {
        rwfs::permissions(this->path(), kReadonlyFilePermissions);
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
    TempFile(const TempDir &dirname) : Temp(dirname.path()) {
    }
    virtual ~TempFile() {
        this->remove();
    }
    virtual void change_perms_normal() const override {
        rwfs::permissions(this->path(),
            rwfs::perms::owner_read | rwfs::perms::owner_write |
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
    template <typename T>
    bool append(T t) const {
        // Append argument at the end of the file.
        // File is open/closes repeatedly. Not optimal.
        std::ofstream ofs(this->path().string(), std::ios::out | std::ios::app);
        ofs << t;
        ofs.close();
        return ofs.good();
    }
    template <typename T>
    bool write(T t) const {
        // Write the argument to the file, discarding all contents.
        // File is open/closes repeatedly. Not optimal.
        std::ofstream ofs(this->path().string(), std::ios::out | std::ios::trunc);
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
    for (const auto &keyValue : map) {
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

BOOST_AUTO_TEST_CASE(test_configParser_initial) {
    // Test an initial config
    [[maybe_unused]] RWConfigParser cfgParser;
}

BOOST_AUTO_TEST_CASE(test_configParser_valid) {
    // Test reading a valid configuration file
    auto cfg = getValidConfig();

    TempFile tempFile;
    tempFile.append(cfg);

    RWConfigParser cfgParser;
    auto [cfgLayer, parseResult] = cfgParser.loadFile(tempFile.path());

    BOOST_CHECK(parseResult.isValid());
    BOOST_CHECK_EQUAL(parseResult.type(),
                      RWConfigParser::ParseResult::GOOD);
    BOOST_CHECK_EQUAL(parseResult.getKeysInvalidData().size(), 0);

    BOOST_REQUIRE(cfgLayer.gamedataPath.has_value());
    BOOST_REQUIRE(cfgLayer.gameLanguage.has_value());
    BOOST_REQUIRE(cfgLayer.invertY.has_value());
    BOOST_REQUIRE(cfgLayer.hudScale.has_value());

    BOOST_CHECK_EQUAL(*cfgLayer.gamedataPath, "/dev/test");
    BOOST_CHECK_EQUAL(*cfgLayer.gameLanguage, "american");
    BOOST_CHECK(*cfgLayer.invertY);
    BOOST_CHECK_EQUAL(*cfgLayer.hudScale, 2.f);
}

BOOST_AUTO_TEST_CASE(test_configParser_valid_modified) {
    // Test reading a valid modified configuration file
    auto cfg = getValidConfig();
    cfg["game"]["path"] = "Liberty City";
    cfg["input"]["invert_y"] = "0";

    TempFile tempFile;
    tempFile.append(cfg);

    RWConfigParser cfgParser;
    auto [cfgLayer, parseResult] = cfgParser.loadFile(tempFile.path());

    BOOST_CHECK(parseResult.isValid());
    BOOST_CHECK_EQUAL(parseResult.type(),
                      RWConfigParser::ParseResult::GOOD);
    BOOST_CHECK_EQUAL(parseResult.getKeysInvalidData().size(), 0);

    BOOST_REQUIRE(cfgLayer.invertY.has_value());
    BOOST_REQUIRE(cfgLayer.gamedataPath.has_value());
    BOOST_CHECK(!*cfgLayer.invertY);
    BOOST_CHECK_EQUAL(*cfgLayer.gamedataPath, "Liberty City");
}

BOOST_AUTO_TEST_CASE(test_configParser_save) {
    // Test saving a configuration file
    auto cfg = getValidConfig();
    cfg["game"]["path"] = "Liberty City";

    TempFile tempFile;
    tempFile.append(cfg);

    {
        RWConfigLayer cfgLayer;
        {
            RWConfigParser cfgParser;
            RWConfigParser::ParseResult parseResult;
            std::tie(cfgLayer, parseResult) = cfgParser.loadFile(tempFile.path());
            BOOST_CHECK(parseResult.isValid());
            BOOST_REQUIRE(cfgLayer.gamedataPath.has_value());
        }

        tempFile.remove();
        BOOST_CHECK(!tempFile.exists());

        {
            RWConfigParser cfgParser;
            auto parseResult = cfgParser.saveFile(tempFile.path(), cfgLayer);
            BOOST_CHECK(parseResult.isValid());
            BOOST_CHECK(tempFile.exists());
        }
    }

    {
        RWConfigParser cfgParser;
        auto [cfgLayer, parseResult] = cfgParser.loadFile(tempFile.path());
        BOOST_CHECK(parseResult.isValid());

        BOOST_REQUIRE(cfgLayer.gamedataPath.has_value());
        BOOST_CHECK_EQUAL(*cfgLayer.gamedataPath, "Liberty City");
    }

    simpleConfig_t cfg2 = readConfig(tempFile.path());
    BOOST_CHECK_EQUAL(cfg2["game"]["path"], "Liberty City");
}

BOOST_AUTO_TEST_CASE(test_configParser_valid_unknown_keys) {
    // Test reading a valid modified configuration file with unknown data
    auto cfg = getValidConfig();
    cfg["game"]["unknownkey"] = "descartes";
    cfg["dontknow"]["dontcare"] = "\t$%!$8847  %%$  ";

    std::map<std::string, std::string> globalUnknownData;

    TempFile tempFile;
    tempFile.append(cfg);

    {
        RWConfigParser cfgParser;
        RWConfigLayer cfgLayer;
        {
            RWConfigParser::ParseResult parseResult;
            std::tie(cfgLayer, parseResult) = cfgParser.loadFile(tempFile.path());

            BOOST_CHECK(parseResult.isValid());

            const auto &unknownData = parseResult.getUnknownData();

            BOOST_CHECK_EQUAL(unknownData.size(), 2);

            BOOST_CHECK_EQUAL(unknownData.count("game.unknownkey"), 1);
            BOOST_CHECK_EQUAL(unknownData.at("game.unknownkey"),
                              stripWhitespace(cfg["game"]["unknownkey"]));

            BOOST_CHECK_EQUAL(unknownData.count("dontknow.dontcare"), 1);
            BOOST_CHECK_EQUAL(unknownData.at("dontknow.dontcare"),
                              stripWhitespace(cfg["dontknow"]["dontcare"]));

            BOOST_CHECK_EQUAL(unknownData.count("game.path"), 0);
            globalUnknownData = unknownData;
        }

        tempFile.remove();

        {
            auto parseResult = cfgParser.saveFile(tempFile.path(), cfgLayer, globalUnknownData);
            BOOST_CHECK(parseResult.isValid());
        }
    }

    {
        RWConfigParser cfgParser;
        auto [cfgLayer, parseResult] = cfgParser.loadFile(tempFile.path());
        BOOST_CHECK(parseResult.isValid());
        const auto &unknownData = parseResult.getUnknownData();

        BOOST_REQUIRE_EQUAL(unknownData.size(), 2);

        BOOST_CHECK_EQUAL(unknownData.count("game.unknownkey"), 1);
        BOOST_CHECK_EQUAL(unknownData.at("game.unknownkey"),
                          stripWhitespace(cfg["game"]["unknownkey"]));

        BOOST_CHECK_EQUAL(unknownData.count("dontknow.dontcare"), 1);
        BOOST_CHECK_EQUAL(unknownData.at("dontknow.dontcare"),
                          stripWhitespace(cfg["dontknow"]["dontcare"]));

        BOOST_CHECK_EQUAL(unknownData.count("game.path"), 0);
    }
}

BOOST_AUTO_TEST_CASE(test_configParser_valid_empty_file) {
    // An empty config file is valid
    TempFile tempFile;
    tempFile.touch();

    RWConfigParser cfgParser;
    auto [cfgLayer, parseResult] = cfgParser.loadFile(tempFile.path());
    BOOST_CHECK(parseResult.isValid());
}

BOOST_AUTO_TEST_CASE(test_configParser_save_readonly) {
    // Test whether saving to a readonly INI file fails
    auto cfg = getValidConfig();

    TempFile tempFile;
    tempFile.append(cfg);
    tempFile.change_perms_readonly();

    RWConfigParser cfgParser;
    RWConfigLayer cfgLayer;
    {
        RWConfigParser::ParseResult parseResult;
        std::tie(cfgLayer, parseResult) = cfgParser.loadFile(tempFile.path());
        BOOST_CHECK(parseResult.isValid());
    }

    {
        auto parseResult = cfgParser.saveFile(tempFile.path(), cfgLayer);
        BOOST_CHECK(!parseResult.isValid());
        BOOST_CHECK_EQUAL(parseResult.type(),
                          RWConfigParser::ParseResult::INVALIDOUTPUTFILE);
    }
}

BOOST_AUTO_TEST_CASE(test_configParser_valid_default) {
    // Test whether the default INI string is valid
    TempFile tempFile;
    BOOST_CHECK(!tempFile.exists());

    RWConfigParser cfgParser;
    {
        auto [cfgLayer, parseResult] = cfgParser.loadFile(tempFile.path());
        BOOST_CHECK(!parseResult.isValid());
    }

    {
        auto defaultLayer = buildDefaultConfigLayer();
        auto parseResult = cfgParser.saveFile(tempFile.path(), defaultLayer);
        BOOST_CHECK(parseResult.isValid());
    }

    BOOST_CHECK(tempFile.exists());

    {
        auto [cfgLayer, parseResult] = cfgParser.loadFile(tempFile.path());
        BOOST_CHECK(parseResult.isValid());
    }
}

BOOST_AUTO_TEST_CASE(test_configParser_invalid_emptykey) {
    // Test duplicate keys in invalid configuration file
    auto cfg = getValidConfig();
    cfg["game"][""] = "0";

    TempFile tempFile;
    tempFile.append(cfg);

    RWConfigParser cfgParser;
    auto [cfgLayer, parseResult] = cfgParser.loadFile(tempFile.path());

    BOOST_CHECK(!parseResult.isValid());
    BOOST_CHECK_EQUAL(parseResult.type(),
                      RWConfigParser::ParseResult::INVALIDINPUTFILE);
}

BOOST_AUTO_TEST_CASE(test_configParser_invalid_duplicate) {
    // Test duplicate keys in invalid configuration file
    auto cfg = getValidConfig();
    cfg["input"]["invert_y    "] = "0";

    TempFile tempFile;
    tempFile.append(cfg);

    RWConfigParser cfgParser;
    auto [cfgLayer, parseResult] = cfgParser.loadFile(tempFile.path());

    BOOST_CHECK(!parseResult.isValid());
    BOOST_CHECK_EQUAL(parseResult.type(),
                      RWConfigParser::ParseResult::INVALIDINPUTFILE);
}

BOOST_AUTO_TEST_CASE(test_configParser_invalid_wrong_type) {
    // Test wrong data type
    auto cfg = getValidConfig();
    cfg["input"]["invert_y"] = "d";

    TempFile tempFile;
    tempFile.append(cfg);

    RWConfigParser cfgParser;
    auto [cfgLayer, parseResult] = cfgParser.loadFile(tempFile.path());

    BOOST_CHECK(!parseResult.isValid());

    BOOST_CHECK_EQUAL(parseResult.type(),
                      RWConfigParser::ParseResult::INVALIDCONTENT);

    BOOST_REQUIRE_EQUAL(parseResult.getKeysInvalidData().size(), 1);

    BOOST_CHECK_EQUAL(parseResult.getKeysInvalidData()[0], "input.invert_y");
}

BOOST_AUTO_TEST_CASE(test_configParser_invalid_nodir) {
    // Test reading non-existing configuration file in non-existing directory
    TempDir tempDir;
    TempFile tempFile(tempDir);

    BOOST_CHECK(!tempDir.exists());
    BOOST_CHECK(!tempFile.exists());

    RWConfigParser cfgParser;
    auto [cfgLayer, parseResult] = cfgParser.loadFile(tempFile.path());

    BOOST_CHECK(!parseResult.isValid());

    BOOST_CHECK_EQUAL(parseResult.type(),
                      RWConfigParser::ParseResult::INVALIDINPUTFILE);
}

BOOST_AUTO_TEST_CASE(test_configParser_invalid_nonexisting) {
    // Test reading non-existing configuration file
    TempFile tempFile;

    BOOST_CHECK(!tempFile.exists());
    RWConfigParser cfgParser;
    auto [cfgLayer, parseResult] = cfgParser.loadFile(tempFile.path());

    BOOST_CHECK(!parseResult.isValid());

    BOOST_CHECK_EQUAL(parseResult.type(),
                      RWConfigParser::ParseResult::INVALIDINPUTFILE);
}

BOOST_AUTO_TEST_CASE(test_argParser_nullptr) {
    RWArgumentParser argParser;
    argParser.parseArguments(0, nullptr);
}

BOOST_AUTO_TEST_CASE(test_argParser_one) {
    RWArgumentParser argParser;
    const char *args[] = {""};
    argParser.parseArguments(1, args);
}

BOOST_AUTO_TEST_CASE(test_argParser_optional_nonexisting) {
    RWArgumentParser argParser;
    const char *args[] = {"", "--nonexistingoptional"};
    auto optLayer = argParser.parseArguments(2, args);
    BOOST_CHECK(!optLayer.has_value());
}

BOOST_AUTO_TEST_CASE(test_argParser_positional_nonexisting) {
    RWArgumentParser argParser;
    const char *args[] = {"", "nonexistingpositional"};
    auto optLayer = argParser.parseArguments(2, args);
    BOOST_CHECK(!optLayer.has_value());
}

BOOST_AUTO_TEST_CASE(test_argParser_bool) {
    RWArgumentParser argParser;
    const char *args[] = {"", "--help"};
    auto optLayer = argParser.parseArguments(2, args);
    BOOST_REQUIRE(optLayer.has_value());
    BOOST_CHECK(optLayer->help);
}

BOOST_AUTO_TEST_CASE(test_argParser_string) {
    RWArgumentParser argParser;
    {
        const auto path = "/some/path";
        const char *args[] = {"", "-c", path};
        auto optLayer = argParser.parseArguments(3, args);
        BOOST_REQUIRE(optLayer.has_value());
        BOOST_REQUIRE(optLayer->configPath.has_value());
        BOOST_CHECK_EQUAL(*optLayer->configPath, path);
    }
    {
        const auto path = "/some/path";
        const char *args[] = {"", "-b", path};
        auto optLayer = argParser.parseArguments(3, args);
        BOOST_REQUIRE(optLayer.has_value());
        BOOST_REQUIRE(optLayer->benchmarkPath.has_value());
        BOOST_CHECK_EQUAL(*optLayer->benchmarkPath, path);
    }
}

BOOST_AUTO_TEST_CASE(test_argParser_int) {
    RWArgumentParser argParser;

    const int width = 1920;
    const auto widthStr = std::to_string(width);
    const char *args[] = {"", "-w", widthStr.c_str()};
    auto optLayer = argParser.parseArguments(3, args);

    BOOST_REQUIRE(optLayer.has_value());
    BOOST_REQUIRE(optLayer->width.has_value());
    BOOST_CHECK_EQUAL(*optLayer->width, width);
}

BOOST_AUTO_TEST_CASE(test_argParser_incomplete_optional) {
    RWArgumentParser argParser;
    const char *args[] = {"", "--hel"};
    auto optLayer = argParser.parseArguments(2, args);
    BOOST_CHECK(!optLayer.has_value());
}

BOOST_AUTO_TEST_CASE(test_argParser_case_sensitive) {
    RWArgumentParser argParser;
    const char *args[] = {"", "--HELP"};
    auto optLayer = argParser.parseArguments(2, args);
    BOOST_CHECK(!optLayer.has_value());
}

BOOST_AUTO_TEST_CASE(test_argParser_int_invalid) {
    RWArgumentParser argParser;

    const auto widthStr = "1920d";
    const char *args[] = {"", "-w", widthStr};
    auto optLayer = argParser.parseArguments(3, args);

    BOOST_CHECK(!optLayer.has_value());
}

BOOST_AUTO_TEST_CASE(test_argParser_bool_newgame) {
    RWArgumentParser argParser;
    {
        const char *args[] = {""};
        auto optLayer = argParser.parseArguments(1, args);

        BOOST_REQUIRE(optLayer.has_value());
        BOOST_CHECK(!optLayer->newGame);
    }
    {
        const char *args[] = {"", "-n"};
        auto optLayer = argParser.parseArguments(2, args);

        BOOST_REQUIRE(optLayer.has_value());
        BOOST_CHECK(optLayer->newGame);
    }
}

BOOST_AUTO_TEST_CASE(test_argParser_bool_invert_y) {
    RWArgumentParser argParser;
    {
        const char *args[] = {""};
        auto optLayer = argParser.parseArguments(1, args);

        BOOST_REQUIRE(optLayer.has_value());
        BOOST_CHECK(!optLayer->invertY.has_value());
    }
    {
        const char *args[] = {"", "--invert_y"};
        auto optLayer = argParser.parseArguments(2, args);

        BOOST_REQUIRE(optLayer.has_value());
        BOOST_REQUIRE(optLayer->invertY.has_value());
        BOOST_CHECK(*optLayer->invertY);
    }
}

BOOST_AUTO_TEST_CASE(test_rwconfig_initial) {
    RWConfig config;
    auto missingKeys = config.missingKeys();
    BOOST_CHECK_NE(missingKeys.size(), 0u);
}

BOOST_AUTO_TEST_CASE(test_rwconfig_defaultLayer) {
    auto defaultLayer = buildDefaultConfigLayer();
    RWConfig config;

    config.setLayer(RWConfig::LAYER_DEFAULT, defaultLayer);
    BOOST_CHECK_NE(config.missingKeys().size(), 0u);
    BOOST_CHECK_EQUAL(config.missingKeys().size(), 1u);

    defaultLayer.gamedataPath = "/path/to/gamedata";
    config.setLayer(RWConfig::LAYER_DEFAULT, defaultLayer);

    BOOST_REQUIRE(config.layers[RWConfig::LAYER_DEFAULT].gamedataPath.has_value());
    BOOST_CHECK_EQUAL(*config.layers[RWConfig::LAYER_DEFAULT].gamedataPath, "/path/to/gamedata");
    BOOST_CHECK_EQUAL(config.gamedataPath(), "/path/to/gamedata");
    BOOST_CHECK_EQUAL(config.missingKeys().size(), 0u);

    config.layers[RWConfig::LAYER_USER].gamedataPath = "/some/other/path/to/gamedata";
    BOOST_REQUIRE(config.layers[RWConfig::LAYER_DEFAULT].gamedataPath.has_value());
    BOOST_CHECK_EQUAL(*config.layers[RWConfig::LAYER_DEFAULT].gamedataPath, "/path/to/gamedata");
    BOOST_REQUIRE(config.layers[RWConfig::LAYER_USER].gamedataPath.has_value());
    BOOST_CHECK_EQUAL(*config.layers[RWConfig::LAYER_USER].gamedataPath, "/some/other/path/to/gamedata");
    BOOST_CHECK_EQUAL(config.gamedataPath(), "/some/other/path/to/gamedata");
}

BOOST_AUTO_TEST_SUITE_END()
