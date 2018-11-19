#ifndef RWGAME_GAMECONFIG_HPP
#define RWGAME_GAMECONFIG_HPP
#include <map>
#include <string>
#include <vector>

#include <rw/filesystem.hpp>

class GameConfig {
private:
    enum ParseType { DEFAULT, CONFIG, FILE, STRING };

    /**
     * @brief extractFilenameParseTypeData Get a human readable filename string
     * @return file path or a description of the data type
     */
    static std::string extractFilenameParseTypeData(ParseType type,
                                                    const std::string &data);

public:
    class ParseResult {
    public:
        enum ErrorType {
            /// UNINITIALIZED: The config was not initialized
            UNINITIALIZED,
            /// GOOD: Input file/string was good
            GOOD,
            /// INVALIDINPUTFILE: There was some error while reading from a file
            /// or string or the input was ambiguous (e.g. duplicate keys)
            INVALIDINPUTFILE,
            /// INVALIDARGUMENT: The parser received impossible arguments
            INVALIDARGUMENT,
            /// INVALIDCONTENT: Some required keys were missing or some values
            /// were of incorrect type
            INVALIDCONTENT,
            /// INVALIDOUTPUTFILE: There was some error while writing to a file
            /// or string
            INVALIDOUTPUTFILE
        };

    private:
        /**
         * @brief ParseResult Holds the issues occurred while parsing of a
         * config file.
         * @param srcType Type of the source
         * @param source The source of the parser
         * @param destType Type of the destination
         * @param destination The destination
         */
        ParseResult(ParseType srcType, const std::string &source,
                    ParseType destType, const std::string &destination);

        /**
         * @brief ParseResult Create empty ParseResult
         */
        ParseResult();

    public:
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
         * @brief Mark this result as valid
         */
        void markGood();

        /**
         * @brief failInputFile Fail because the input file was invalid
         * @param line Line number where the error is located
         * @param message Description of the error
         */
        void failInputFile(size_t line, const std::string &message);

        /**
         * @brief failArgument Fail because an argument was invalid
         * @param srcType type of the source
         * @param destType type of the destination
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
         * @brief failOutputFile Fail because an error occurred while while
         * writing to the output
         * @param line Line number where the error is located
         * @param message Description of the error
         */
        void failOutputFile(size_t line, const std::string &message);

        /**
         * @brief isValid
         * @return True if the loaded configuration is valid
         */
        bool isValid() const;

        /**
         * @brief what Get a string representing the error
         * @return String with the error description
         */
        std::string what() const;

        /**
         * @brief addUnknownData Add unknown key value pairs
         * @param key The unknown key
         * @param value The associated data
         */
        void addUnknownData(const std::string &key, const std::string &value);

        /**
         * @brief addUnknownData Get all the unknown key value pairs
         * @return Mapping of the unknown keys with associated data
         */
        const std::map<std::string, std::string> &getUnknownData() const;

    private:
        /// Type of the failure
        ErrorType m_result;

        /// Filename of the input file
        std::string m_inputfilename{};

        /// Filename of the output file
        std::string m_outputfilename{};

        /// Line number where the failure occurred (on invalid input or output
        /// file)
        size_t m_line{};

        /// Description of the failure (on invalid input or output file)
        std::string m_message;

        /// All required keys that are missing
        std::vector<std::string> m_keys_requiredMissing;

        /// All keys that contain invalid data
        std::vector<std::string> m_keys_invalidData;

        // Mapping of unknown keys and associated data
        std::map<std::string, std::string> m_unknownData;

        /**
         * @brief setUnknownData Replace the the unknown key value pairs
         */
        void setUnknownData(
            const std::map<std::string, std::string> &unknownData);

        friend class GameConfig;
    };

    /**
     * @brief GameConfig Create a game configuration (initially invalid)
     */
    GameConfig() = default;

    /**
     * @brief Initialize this object using the config file at path
     * @param path Path of the configuration file
     */
    void loadFile(const rwfs::path &path);

    /**
     * @brief getConfigPath Returns the path for the configuration
     */
    rwfs::path getConfigPath() const;

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
     * @brief getConfigString Returns the content of the default INI
     * configuration.
     * @return INI string
     */
    std::string getDefaultINIString();

    const rwfs::path &getGameDataPath() const {
        return m_gamePath;
    }
    const std::string &getGameLanguage() const {
        return m_gameLanguage;
    }
    bool getInputInvertY() const {
        return m_inputInvertY;
    }
    int getWindowWidth() const {
        return m_windowWidth;
    }
    int getWindowHeight() const {
        return m_windowHeight;
    }
    bool getWindowFullscreen() const {
        return m_windowFullscreen;
    }
    float getHUDScale() const {
        return m_HUDscale;
    }

    static rwfs::path getDefaultConfigPath();
private:

    /**
     * @brief parseConfig Load data from source and write it to destination.
     * Whitespace will be stripped from unknown data.
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
    rwfs::path m_configPath{};
    ParseResult m_parseResult{};

    /* Actual Configuration */

    /// Path to the game data
    rwfs::path m_gamePath;

    /// Language for game
    std::string m_gameLanguage = "american";

    /// Invert the y axis for camera control.
    bool m_inputInvertY = false;

    /// Size of the window
    int m_windowWidth{800};
    int m_windowHeight{600};
    
    /// Set the window to fullscreen
    bool m_windowFullscreen = false;

    /// HUD scale parameter 
    float m_HUDscale = 1.f;
};

#endif
