#ifndef RWGAME_GAMECONFIG_HPP
#define RWGAME_GAMECONFIG_HPP
#include <string>
#include <vector>

class GameConfig {
public:
    class ParseResult {
    public:
        enum ErrorType {
            /// GOOD: Input file/string was good
            GOOD,
            /// INVALIDINPUTFILE: There was some error while reading from a file or string or the input was ambiguous (e.g. duplicate keys)
            INVALIDINPUTFILE,
            /// INVALIDARGUMENT: The parser received impossible arguments
            INVALIDARGUMENT,
            /// INVALIDCONTENT: Some required keys were missing or some values were of incorrect type
            INVALIDCONTENT,
            /// INVALIDOUTPUTFILE: There was some error while writing to a file or string
            INVALIDOUTPUTFILE
        };
        /**
         * @brief ParseResult Holds the issues occurred while parsing of a config file.
         */

        ParseResult();
        /**
         * @brief type Get the type of error
         * @return Type of error or GOOD if there was no error
         */
        ErrorType type() const;

        /**
         * @brief getKeysRequiredMissing Get the keys that were missing
         * @return A vector with all the keys
         */
        const std::vector<std::string> &getKeysRequiredMissing() const;

        /**
         * @brief getKeysInvalidData Get the keys that contained invalid data
         * @return A vector with all the keys
         */
        const std::vector<std::string> &getKeysInvalidData() const;
        /**
         * @brief failInputFile Fail because the input file was invalid
         * @param filename Filename of the invalid file
         * @param line Line number where the error is located
         * @param message Description of the error
         */
        void failInputFile(const std::string &filename, size_t line, const std::string &message);

        /**
         * @brief failArgument Fail because an argument was invalid
         */
        void failArgument();

        /**
         * @brief failRequiredMissing Fail because a required key is missing
         * @param key The key that is missing
         */
        void failRequiredMissing(const std::string &key);

        /**
         * @brief failInvalidData Fail because a key contains invalid data
         * @param key The key that contains invalid data
         */
        void failInvalidData(const std::string &key);

        /**
         * @brief failOutputFile Fail because an error occurred while while writing to the output
         * @param filename Filename of the invalid file
         * @param line Line number where the error is located
         * @param message Description of the error
         */
        void failOutputFile(const std::string &filename, size_t line, const std::string &message);

        /**
         * @brief isValid
         * @return True if the loaded configuration is valid
         */
        bool isValid() const;
    private:
        /// Type of the failure
        ErrorType m_result;

        /// Filename of the invalid input or output file
        std::string m_filename;

        /// Line number where the failure occurred (on invalid input or output file)
        size_t m_line;

        /// Description of the failure (on invalid input or output file)
        std::string m_message;

        /// All required keys that are missing
        std::vector<std::string> m_keys_requiredMissing;

        /// All keys that contain invalid data
        std::vector<std::string> m_keys_invalidData;
    };

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
     * @brief writeConfig Save the game configuration
     */
    ParseResult saveConfig();

    /**
     * @brief isValid
     * @return True if the loaded configuration is valid
     */
    bool isValid() const;

    /**
     * @brief getParseResult Get more information on parsing failures
     * @return A ParseResult object  containing more information
     */
    const ParseResult &getParseResult() const;

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
    ParseResult parseConfig(ParseType srcType, const std::string &source,
        ParseType destType, std::string &destination);

    /* Config State */
    std::string m_configName;
    std::string m_configPath;
    ParseResult m_parseResult;

    /* Actual Configuration */

    /// Path to the game data
    std::string m_gamePath;

    /// Language for game
    std::string m_gameLanguage = "american";

    /// Invert the y axis for camera control.
    bool m_inputInvertY;
};

#endif
