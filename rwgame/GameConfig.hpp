#ifndef RWGAME_GAMECONFIG_HPP
#define RWGAME_GAMECONFIG_HPP
#include <string>

class GameConfig {
public:
    /**
     * @brief GameConfig Loads a game configuration
     * @param configName The configuration filename to load
     * @param configPath Where to look.
     */
    GameConfig(const std::string& configName,
               const std::string& configPath = getDefaultConfigPath());

    /**
     * @brief getFilePath Returns the system file path for the configuration
     */
    std::string getConfigFile() const;

    /**
     * @brief isValid
     * @return True if the loaded configuration is valid
     */
    bool isValid() const;

    /**
     * @brief getConfigString Returns the content of the default INI configuration.
     * @return INI string
     */
    std::string getDefaultINIString();

    const std::string& getGameDataPath() const {
        return m_gamePath;
    }
    const std::string& getGameLanguage() const {
        return m_gameLanguage;
    }
    bool getInputInvertY() const {
        return m_inputInvertY;
    }

private:
    static std::string getDefaultConfigPath();

    enum ParseType {
        DEFAULT,
        CONFIG,
        FILE,
        STRING
    };

    /**
     * @brief parseConfig Load data from source and write it to destination.
     * @param srcType Can be DEFAULT | CONFIG | FILE | STRING
     * @param source don't care if srcType == (DEFAULT | CONFIG),
     *               path of INI file if srcType == FILE
     *               INI string if srcType == STRING
     * @param destType Can be CONFIG | FILE | STRING (DEFAULT is invalid)
     * @param destination don't care if srcType == CONFIG
     *                    path of INI file if destType == FILE
     *                    INI string if srcType == STRING
     * @return True if the parsing succeeded
     */
    bool parseConfig(ParseType srcType, const std::string &source,
        ParseType destType, std::string &destination);

    /* Config State */
    std::string m_configName;
    std::string m_configPath;
    bool m_valid;

    /* Actual Configuration */

    /// Path to the game data
    std::string m_gamePath;

    /// Language for game
    std::string m_gameLanguage = "american";

    /// Invert the y axis for camera control.
    bool m_inputInvertY;
};

#endif
