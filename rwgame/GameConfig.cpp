#include "GameConfig.hpp"
#include <cstdlib>
#include <cstring>
#include <rw/defines.hpp>

#include <ini.h>

const std::string kConfigDirectoryName("OpenRW");

GameConfig::GameConfig(const std::string& configName,
                       const std::string& configPath)
    : m_configName(configName)
    , m_configPath(configPath)
    , m_valid(false)
    , m_inputInvertY(false) {
    if (m_configPath.empty()) {
        m_configPath = getDefaultConfigPath();
    }

    // Look up the path to use
    auto configFile = getConfigFile();

    if (ini_parse(configFile.c_str(), handler, this) < 0) {
        m_valid = false;
    } else {
        m_valid = true;
    }
}

std::string GameConfig::getConfigFile() {
    return m_configPath + "/" + m_configName;
}

bool GameConfig::isValid() {
    return m_valid;
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

int GameConfig::handler(void* user, const char* section, const char* name,
                        const char* value) {
    auto self = static_cast<GameConfig*>(user);
#define MATCH(_s, _n) (strcmp(_s, section) == 0 && strcmp(_n, name) == 0)

    if (MATCH("game", "path")) {
        self->m_gamePath = value;
    } else if (MATCH("game", "language")) {
        // @todo Don't allow path seperators and relative directories
        self->m_gameLanguage = value;
    } else if (MATCH("input", "invert_y")) {
        self->m_inputInvertY = atoi(value) > 0;
    } else {
        RW_MESSAGE("Unhandled config entry [" << section << "] " << name
                                              << " = " << value);
        return 0;
    }

    return 1;

#undef MATCH
}
