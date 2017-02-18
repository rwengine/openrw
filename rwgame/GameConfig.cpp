#include "GameConfig.hpp"
#include <cstdlib>
#include <cstring>
#include <rw/defines.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
namespace pt = boost::property_tree;

const std::string kConfigDirectoryName("OpenRW");

GameConfig::GameConfig(const std::string& configName,
                       const std::string& configPath)
    : m_configName(configName)
    , m_configPath(configPath)
    , m_parseResult()
    , m_inputInvertY(false) {
    if (m_configPath.empty()) {
        m_configPath = getDefaultConfigPath();
    }

    // Look up the path to use
    auto configFile = getConfigFile();

    std::string dummy;
    m_parseResult = parseConfig(ParseType::FILE, configFile, ParseType::CONFIG, dummy);
}

std::string GameConfig::getConfigFile() const {
    return m_configPath + "/" + m_configName;
}

bool GameConfig::isValid() const {
    return m_parseResult.isValid();
}

const GameConfig::ParseResult &GameConfig::getParseResult() const {
    return m_parseResult;
}

std::string GameConfig::getDefaultConfigPath() {
#if defined(RW_LINUX) || defined(RW_FREEBSD) || defined(RW_NETBSD) || \
    defined(RW_OPENBSD)
    char* config_home = getenv("XDG_CONFIG_HOME");
    if (config_home != nullptr) {
        return std::string(config_home) + "/" + kConfigDirectoryName;
    }
    char* home = getenv("HOME");
    if (home != nullptr) {
        return std::string(home) + "/.config/" + kConfigDirectoryName;
    }

#elif defined(RW_OSX)
    char* home = getenv("HOME");
    if (home)
        return std::string(home) + "/Library/Preferences/" +
               kConfigDirectoryName;

#else
    return ".";
#endif

    // Well now we're stuck.
    RW_ERROR("No default config path found.");
    return ".";
}

std::string stripComments(const std::string &str) {
    auto s = std::string(str, 0, str.find_first_of(";#"));
    return s.erase(s.find_last_not_of(" \n\r\t")+1);
}

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
    typedef bool        external_type;
    boost::optional<external_type> get_value(const internal_type &str) {
        boost::optional<external_type> res;
        try {
            res = std::stoi(stripComments(str)) != 0;
        } catch (std::invalid_argument &e) {
        }
        return res;
    }
    boost::optional<internal_type> put_value(const external_type &b) {
        return internal_type(b ? "1" : "0");
    }
};

struct IntTranslator {
    typedef std::string internal_type;
    typedef int         external_type;
    boost::optional<external_type> get_value(const internal_type &str) {
        boost::optional<external_type> res;
        try {
            res = std::stoi(stripComments(str));
        } catch (std::invalid_argument &e) {
        }
        return res;
    }
    boost::optional<internal_type> put_value(const external_type &i) {
        return std::to_string(i);
    }
};

GameConfig::ParseResult GameConfig::saveConfig() {
    auto filename = getConfigFile();
    return parseConfig(ParseType::CONFIG, "",
        ParseType::FILE, filename);
}

std::string GameConfig::getDefaultINIString() {
    std::string result;
    parseConfig(ParseType::DEFAULT, "", ParseType::STRING, result);
    return result;
}

GameConfig::ParseResult GameConfig::parseConfig(
    GameConfig::ParseType srcType, const std::string &source,
    ParseType destType, std::string &destination)
{
    pt::ptree srcTree;
    ParseResult parseResult(srcType, source, destType, destination);

    try {
        if (srcType == ParseType::STRING) {
            pt::read_ini(source, srcTree);
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

    auto read_config = [&](const std::string &key, auto &target,
                          const auto &defaultValue, auto &translator,
                          bool optional=true) {
        typedef typename std::remove_reference<decltype(target)>::type config_t;

        config_t sourceValue;

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
                    // Catches missing key-value pairs: fail when required
                    if (!optional) {
                        parseResult.failRequiredMissing(key);
                        RW_MESSAGE(e.what());
                        return;
                    }
                    sourceValue = defaultValue;
                } catch (pt::ptree_bad_data &e) {
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

    // Add new configuration parameters here.
    // Additionally, add them to the unit test.

    // @todo Don't allow path separators and relative directories
    read_config("game.path", this->m_gamePath, "/opt/games/Grand Theft Auto 3", deft, false);
    read_config("game.language", this->m_gameLanguage, "american", deft);

    read_config("input.invert_y", this->m_inputInvertY, false, boolt);

    if (!parseResult.isValid())
        return parseResult;

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

    return parseResult;
}

std::string GameConfig::extractFilenameParseTypeData(ParseType type, const std::string &data)
{
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

GameConfig::ParseResult::ParseResult(
            GameConfig::ParseType srcType, const std::string &source,
            GameConfig::ParseType destType, const std::string &destination)
    : m_result(ErrorType::GOOD)
    , m_inputfilename(GameConfig::extractFilenameParseTypeData(srcType, source))
    , m_outputfilename(GameConfig::extractFilenameParseTypeData(destType, destination))
    , m_line(0)
    , m_message()
    , m_keys_requiredMissing()
    , m_keys_invalidData() {
}

GameConfig::ParseResult::ParseResult()
    : m_result(ErrorType::GOOD)
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

const std::vector<std::string> &GameConfig::ParseResult::getKeysRequiredMissing() const {
    return this->m_keys_requiredMissing;
}

const std::vector<std::string> &GameConfig::ParseResult::getKeysInvalidData() const {
    return this->m_keys_invalidData;
}

std::string GameConfig::ParseResult::what() const {
    switch (this->m_result) {
        case ErrorType::GOOD:
            return "Good";
        case ErrorType::INVALIDARGUMENT:
            return "Invalid argument: destination cannot be the default config";
        case ErrorType::INVALIDINPUTFILE:
        {
            std::ostringstream oss;
            oss << "Error while reading \"" 
                << this->m_inputfilename << "\":" << this->m_line << ":\n"
                << this->m_message;
            return oss.str();
        }
        case ErrorType::INVALIDOUTPUTFILE:
        {
            std::ostringstream oss;
            oss << "Error while writing \"" 
                << this->m_inputfilename << "\":" << this->m_line << ":\n"
                << this->m_message;
            return oss.str();
        }
        case ErrorType::INVALIDCONTENT:
        {
            std::ostringstream oss;
            oss << "Error while parsing \"" 
                << this->m_inputfilename << "\".";
            if (this->m_keys_requiredMissing.size()) {
                oss << "\nRequired keys that are missing:";
                for (auto &key : this->m_keys_requiredMissing) {
                    oss << "\n - " << key;
                }
            }
            if (this->m_keys_invalidData.size()) {
                oss << "\nKeys that contain invalid data:";
                for (auto &key : this->m_keys_invalidData) {
                    oss << "\n - " << key;
                }
            }
            return oss.str();
        }
        default:
            return "Unknown error";
    }
}
