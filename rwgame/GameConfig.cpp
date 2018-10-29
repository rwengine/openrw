#include "GameConfig.hpp"

#include <rw/debug.hpp>
#include <rw/filesystem.hpp>

#include <algorithm>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
namespace pt = boost::property_tree;

#ifdef RW_WINDOWS
#include <Shlobj.h>
#include <winerror.h>

#include <platform/RWWindows.hpp>
#endif

const std::string kConfigDirectoryName("OpenRW");

void GameConfig::loadFile(const rwfs::path &path) {
    m_configPath = path;
    std::string dummy;
    m_parseResult =
        parseConfig(ParseType::FILE, path.string(), ParseType::CONFIG, dummy);
}

rwfs::path GameConfig::getConfigPath() const {
    return m_configPath;
}

bool GameConfig::isValid() const {
    return m_parseResult.isValid();
}

const GameConfig::ParseResult &GameConfig::getParseResult() const {
    return m_parseResult;
}

rwfs::path GameConfig::getDefaultConfigPath() {
#if defined(RW_LINUX) || defined(RW_FREEBSD) || defined(RW_NETBSD) || \
    defined(RW_OPENBSD)
    char *config_home = getenv("XDG_CONFIG_HOME");
    if (config_home != nullptr) {
        return rwfs::path(config_home) / kConfigDirectoryName;
    }
    char *home = getenv("HOME");
    if (home != nullptr) {
        return rwfs::path(home) / ".config/" / kConfigDirectoryName;
    }

#elif defined(RW_OSX)
    char *home = getenv("HOME");
    if (home)
        return rwfs::path(home) / "Library/Preferences/" / kConfigDirectoryName;

#elif defined(RW_WINDOWS)
    wchar_t *widePath;
    auto res = SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_DEFAULT,
                                    nullptr, &widePath);
    if (SUCCEEDED(res)) {
        auto utf8Path = wideStringToACP(widePath);
        return rwfs::path(utf8Path) / kConfigDirectoryName;
    }
#else
    return rwfs::path();
#endif

    // Well now we're stuck.
    RW_ERROR("No default config path found.");
    return rwfs::path();
}

std::string stripComments(const std::string &str) {
    auto s = std::string(str, 0, str.find_first_of(";#"));
    return s.erase(s.find_last_not_of(" \n\r\t") + 1);
}

struct PathTranslator {
    typedef std::string internal_type;
    typedef rwfs::path external_type;
    boost::optional<external_type> get_value(const internal_type &str) {
        return rwfs::path(str);
    }
    boost::optional<internal_type> put_value(const external_type &path) {
        return path.string();
    }
};

struct StringTranslator {
    typedef std::string internal_type;
    typedef std::string external_type;
    boost::optional<external_type> get_value(const internal_type &str) {
        return stripComments(str);
    }
    boost::optional<internal_type> put_value(const external_type &str) {
        return str;
    }
};

struct BoolTranslator {
    typedef std::string internal_type;
    typedef bool external_type;
    boost::optional<external_type> get_value(const internal_type &str) {
        boost::optional<external_type> res;
        try {
            res = std::stoi(stripComments(str)) != 0;
        } catch (std::invalid_argument &) {
        }
        return res;
    }
    boost::optional<internal_type> put_value(const external_type &b) {
        return internal_type(b ? "1" : "0");
    }
};

struct IntTranslator {
    typedef std::string internal_type;
    typedef int external_type;
    boost::optional<external_type> get_value(const internal_type &str) {
        boost::optional<external_type> res;
        try {
            res = std::stoi(stripComments(str));
        } catch (std::invalid_argument &) {
        }
        return res;
    }
    boost::optional<internal_type> put_value(const external_type &i) {
        return std::to_string(i);
    }
};

struct FloatTranslator {
    typedef std::string internal_type;
    typedef float external_type;
    boost::optional<external_type> get_value(const internal_type &str) {
        boost::optional<external_type> res;
        try {
            res = std::stof(stripComments(str));
        } catch (std::invalid_argument &) {
        }
        return res;
    }
    boost::optional<internal_type> put_value(const external_type &f) {
        return std::to_string(f);
    }
};

GameConfig::ParseResult GameConfig::saveConfig() {
    auto configPath = getConfigPath().string();
    return parseConfig(ParseType::CONFIG, "", ParseType::FILE, configPath);
}

std::string GameConfig::getDefaultINIString() {
    std::string result;
    parseConfig(ParseType::DEFAULT, "", ParseType::STRING, result);
    return result;
}

GameConfig::ParseResult GameConfig::parseConfig(GameConfig::ParseType srcType,
                                                const std::string &source,
                                                ParseType destType,
                                                std::string &destination) {
    // srcTree: holds all key/value pairs
    pt::ptree srcTree;
    ParseResult parseResult(srcType, source, destType, destination);

    try {
        if (srcType == ParseType::STRING) {
            std::istringstream iss(source);
            pt::read_ini(iss, srcTree);
        } else if (srcType == ParseType::FILE) {
            pt::read_ini(source, srcTree);
        }
    } catch (pt::ini_parser_error &e) {
        // Catches illegal input files (nonsensical input, duplicate keys)
        parseResult.failInputFile(e.line(), e.message());
        RW_MESSAGE(e.what());
        return parseResult;
    }

    if (destType == ParseType::DEFAULT) {
        parseResult.failArgument();
        RW_ERROR("Target cannot be DEFAULT.");
        return parseResult;
    }

    // knownKeys: holds all known keys
    std::vector<std::string> knownKeys;

    auto read_config = [&](const std::string &key, auto &target,
                           const auto &defaultValue, auto &translator,
                           bool optional = true) {
        typedef typename std::remove_reference<decltype(target)>::type config_t;

        config_t sourceValue;
        knownKeys.push_back(key);

        switch (srcType) {
            case ParseType::DEFAULT:
                sourceValue = defaultValue;
                break;
            case ParseType::CONFIG:
                sourceValue = target;
                break;
            case ParseType::FILE:
            case ParseType::STRING:
                try {
                    sourceValue = srcTree.get<config_t>(key, translator);
                } catch (pt::ptree_bad_path &e) {
                    RW_UNUSED(e);
                    // Catches missing key-value pairs: fail when required
                    if (!optional) {
                        parseResult.failRequiredMissing(key);
                        RW_MESSAGE(e.what());
                        return;
                    }
                    sourceValue = defaultValue;
                } catch (pt::ptree_bad_data &e) {
                    RW_UNUSED(e);
                    // Catches illegal value data: always fail
                    parseResult.failInvalidData(key);
                    RW_MESSAGE(e.what());
                    return;
                }
                break;
        }
        srcTree.put(key, sourceValue, translator);

        switch (destType) {
            case ParseType::DEFAULT:
                // Target cannot be DEFAULT (case already handled)
                parseResult.failArgument();
                break;
            case ParseType::CONFIG:
                // Don't care if success == false
                target = sourceValue;
                break;
            case ParseType::FILE:
            case ParseType::STRING:
                break;
        }
    };

    auto deft = StringTranslator();
    auto boolt = BoolTranslator();
    auto patht = PathTranslator();
    auto intt = IntTranslator();
    auto floatt = FloatTranslator();

    // Add new configuration parameters here.
    // Additionally, add them to the unit test.

    // @todo Don't allow path separators and relative directories
    read_config("game.path", this->m_gamePath, "/opt/games/Grand Theft Auto 3",
                patht, false);
    read_config("game.language", this->m_gameLanguage, "american", deft);
    read_config("game.hud_scale", this->m_HUDscale, 1.f, floatt);

    read_config("input.invert_y", this->m_inputInvertY, false, boolt);

    read_config("window.width", this->m_windowWidth, 800, intt);
    read_config("window.height", this->m_windowHeight, 600, intt);
    read_config("window.fullscreen", this->m_windowFullscreen, false, boolt);

    // Build the unknown key/value map from the correct source
    switch (srcType) {
        case ParseType::FILE:
        case ParseType::STRING:
            for (const auto &section : srcTree) {
                for (const auto &subKey : section.second) {
                    std::string key = section.first + "." + subKey.first;
                    if (std::find(knownKeys.begin(), knownKeys.end(), key) ==
                        knownKeys.end()) {
                        RW_MESSAGE("Unknown configuration key: " << key);
                        parseResult.addUnknownData(key, subKey.second.data());
                    }
                }
            }
            break;
        case ParseType::CONFIG:
            parseResult.setUnknownData(m_parseResult.getUnknownData());
            break;
        case ParseType::DEFAULT:
            break;
    }

    // Store the unknown key/value map to the correct destination
    switch (destType) {
        case ParseType::CONFIG:
            m_parseResult.setUnknownData(parseResult.getUnknownData());
            break;
        case ParseType::STRING:
        case ParseType::FILE:
            for (const auto &keyvalue : parseResult.getUnknownData()) {
                srcTree.put(keyvalue.first, keyvalue.second);
            }
            break;
        default:
            break;
    }

    if (!parseResult.isValid()) return parseResult;

    try {
        if (destType == ParseType::STRING) {
            std::ostringstream ostream;
            pt::write_ini(ostream, srcTree);
            destination = ostream.str();
        } else if (destType == ParseType::FILE) {
            pt::write_ini(destination, srcTree);
        }
    } catch (pt::ini_parser_error &e) {
        parseResult.failOutputFile(e.line(), e.message());
        RW_MESSAGE(e.what());
    }

    if (parseResult.type() == ParseResult::ErrorType::UNINITIALIZED) {
        parseResult.markGood();
    }

    return parseResult;
}

std::string GameConfig::extractFilenameParseTypeData(ParseType type,
                                                     const std::string &data) {
    switch (type) {
        case ParseType::CONFIG:
            return "<configuration>";
        case ParseType::FILE:
            return data;
        case ParseType::STRING:
            return "<string>";
        case ParseType::DEFAULT:
        default:
            return "<default>";
    }
}

GameConfig::ParseResult::ParseResult(GameConfig::ParseType srcType,
                                     const std::string &source,
                                     GameConfig::ParseType destType,
                                     const std::string &destination)
    : m_result(ErrorType::GOOD)
    , m_inputfilename(GameConfig::extractFilenameParseTypeData(srcType, source))
    , m_outputfilename(
          GameConfig::extractFilenameParseTypeData(destType, destination))
    , m_line(0)
    , m_message()
    , m_keys_requiredMissing()
    , m_keys_invalidData()
    , m_unknownData() {
}

GameConfig::ParseResult::ParseResult()
    : m_result(ErrorType::UNINITIALIZED)
    , m_inputfilename()
    , m_outputfilename()
    , m_line(0)
    , m_message()
    , m_keys_requiredMissing()
    , m_keys_invalidData() {
}

GameConfig::ParseResult::ErrorType GameConfig::ParseResult::type() const {
    return this->m_result;
}

bool GameConfig::ParseResult::isValid() const {
    return this->type() == ErrorType::GOOD;
}

void GameConfig::ParseResult::failInputFile(size_t line,
                                            const std::string &message) {
    this->m_result = ParseResult::ErrorType::INVALIDINPUTFILE;
    this->m_line = line;
    this->m_message = message;
}

void GameConfig::ParseResult::markGood() {
    this->m_result = ParseResult::ErrorType::GOOD;
}

void GameConfig::ParseResult::failArgument() {
    this->m_result = ParseResult::ErrorType::INVALIDARGUMENT;
}

void GameConfig::ParseResult::failRequiredMissing(const std::string &key) {
    this->m_result = ParseResult::ErrorType::INVALIDCONTENT;
    this->m_keys_requiredMissing.push_back(key);
}

void GameConfig::ParseResult::failInvalidData(const std::string &key) {
    this->m_result = ParseResult::ErrorType::INVALIDCONTENT;
    this->m_keys_invalidData.push_back(key);
}

void GameConfig::ParseResult::failOutputFile(size_t line,
                                             const std::string &message) {
    this->m_result = ParseResult::ErrorType::INVALIDOUTPUTFILE;
    this->m_line = line;
    this->m_message = message;
}

const std::vector<std::string>
    &GameConfig::ParseResult::getKeysRequiredMissing() const {
    return this->m_keys_requiredMissing;
}

const std::vector<std::string> &GameConfig::ParseResult::getKeysInvalidData()
    const {
    return this->m_keys_invalidData;
}

std::string GameConfig::ParseResult::what() const {
    std::ostringstream oss;
    switch (this->m_result) {
        case ErrorType::UNINITIALIZED:
            oss << "Parsing was skipped or did not finish.";
            break;
        case ErrorType::GOOD:
            oss << "Parsing completed without errors.";
            break;
        case ErrorType::INVALIDARGUMENT:
            oss << "Invalid argument: destination cannot be the default "
                   "config.";
            break;
        case ErrorType::INVALIDINPUTFILE:
            oss << "Error while reading \"" << this->m_inputfilename
                << "\":" << this->m_line << ":\n"
                << this->m_message << ".";
            break;
        case ErrorType::INVALIDOUTPUTFILE:
            oss << "Error while writing \"" << this->m_inputfilename
                << "\":" << this->m_line << ":\n"
                << this->m_message << ".";
            break;
        case ErrorType::INVALIDCONTENT:
            oss << "Error while parsing \"" << this->m_inputfilename << "\".";
            if (!this->m_keys_requiredMissing.empty()) {
                oss << "\nRequired keys that are missing:";
                for (auto &key : this->m_keys_requiredMissing) {
                    oss << "\n - " << key;
                }
            }
            if (!this->m_keys_invalidData.empty()) {
                oss << "\nKeys that contain invalid data:";
                for (auto &key : this->m_keys_invalidData) {
                    oss << "\n - " << key;
                }
            }
            break;
        default:
            oss << "Unknown error.";
            break;
    }
    if (!this->m_unknownData.empty()) {
        oss << "\nUnknown configuration keys:";
        for (const auto &[key, value] : m_unknownData) {
            oss << "\n - " << key;
        }
    }
    return oss.str();
}

void GameConfig::ParseResult::addUnknownData(const std::string &key,
                                             const std::string &value) {
    this->m_unknownData[key] = value;
}

const std::map<std::string, std::string>
    &GameConfig::ParseResult::getUnknownData() const {
    return this->m_unknownData;
}

void GameConfig::ParseResult::setUnknownData(
    const std::map<std::string, std::string> &unknownData) {
    this->m_unknownData = unknownData;
}
