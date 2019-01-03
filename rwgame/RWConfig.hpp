#ifndef RWGAME_RWCONFIG_HPP
#define RWGAME_RWCONFIG_HPP

#include <boost/program_options.hpp>

#include <array>
#include <filesystem>
#include <iosfwd>
#include <map>
#include <optional>
#include <string>
#include <tuple>
#include <vector>


struct RWConfigLayer {
#define RWCONFIGARG(_RW_TYPE, _RW_NAME, _RW_DEFAULT, _RW_CONFPATH, _RW_CATEGORY, _RW_ARGMASK, _RW_ARGMETA, _RW_HELP) \
    std::optional<_RW_TYPE> _RW_NAME;
#define RWARG(...)
#define RWARG_OPT(...)
#include "RWConfig.inc"
#undef RWARG_OPT
#undef RWARG
#undef RWCONFIGARG
};

struct RWArgConfigLayer : public RWConfigLayer {
#define RWCONFIGARG(...)
#define RWARG(_RW_TYPE, _RW_NAME, _RW_CATEGORY, _RW_ARGMASK, _RW_ARGMETA, _RW_HELP) \
    _RW_TYPE _RW_NAME;
#define RWARG_OPT(_RW_TYPE, _RW_NAME, _RW_CATEGORY, _RW_ARGMASK, _RW_ARGMETA, _RW_HELP) \
    std::optional<_RW_TYPE> _RW_NAME;
#include "RWConfig.inc"
#undef RWARG_OPT
#undef RWARG
#undef RWCONFIGARG
};

RWConfigLayer buildDefaultConfigLayer();

template <size_t N>
class RWConfigLayers {
    template <typename T, typename F>
    std::optional<T> get(F &&func) const {
        for (const auto & layer : layers) {
            std::optional<T> optValue = func(layer);
            if (optValue.has_value()) {
                return optValue;
            }
        }
        return std::nullopt;
    }
public:
    std::array<RWConfigLayer, N> layers;
    template <typename Layer>
    void setLayer(size_t i, Layer&& layer) {
        layers[i] = std::forward<Layer>(layer);
    }
#define RWCONFIGARG(_RW_TYPE, _RW_NAME, _RW_DEFAULT, _RW_CONFPATH, _RW_CATEGORY, _RW_ARGMASK, _RW_ARGMETA, _RW_HELP)    \
    _RW_TYPE _RW_NAME() const {                                                                                         \
        return *get<_RW_TYPE>([](auto && l) { return l._RW_NAME;});                                                     \
    }
#define RWARG(...)
#define RWARG_OPT(...)
#include "RWConfig.inc"
#undef RWARG_OPT
#undef RWARG
#undef RWCONFIGARG
    std::vector<std::string> missingKeys() const {
        std::vector<std::string> missing;
#define RWCONFIGARG(_RW_TYPE, _RW_NAME, _RW_DEFAULT, _RW_CONFPATH, _RW_CATEGORY, _RW_ARGMASK, _RW_ARGMETA, _RW_HELP)    \
        if (!get<_RW_TYPE>([](auto && l) { return l._RW_NAME;}).has_value()) {                                          \
            missing.push_back(_RW_CONFPATH);                                                                            \
        }
#define RWARG(...)
#define RWARG_OPT(...)
#include "RWConfig.inc"
#undef RWARG_OPT
#undef RWARG
#undef RWCONFIGARG
        return missing;
    }
};

class RWConfig : public RWConfigLayers<4> {
public:
    enum {
        LAYER_USER = 0,
        LAYER_ARGUMENT = 1,
        LAYER_CONFIGFILE = 2,
        LAYER_DEFAULT = 3,
    };
    std::map<std::string, std::string> unknown;
};

class RWArgumentParser {
    boost::program_options::options_description _desc;
public:
    enum Category {
        CONFIG,
        GAME,
        INPUT,
        WINDOW,
        DEVELOP,
        GENERAL,
        COUNT_,
    };
    RWArgumentParser();
    RWArgumentParser(const RWArgumentParser& parser) = default;
    RWArgumentParser(RWArgumentParser&& parser) = default;
    std::ostream &printHelp(std::ostream &os) const;
    std::optional<RWArgConfigLayer> parseArguments(int argc, const char* argv[]) const;  // FIXME(madebr): change to const char?
};

class RWConfigParser {

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
            /// INVALIDCONTENT: Some required keys were missing or some values
            /// were of incorrect type
            INVALIDCONTENT,
            /// INVALIDOUTPUTFILE: There was some error while writing to a file
            /// or string
            INVALIDOUTPUTFILE
        };

        /**
         * @brief ParseResult Create empty ParseResult
         */
        ParseResult() = default;

    private:
        /**
         * @brief ParseResult holds the issues occurred while parsing of a
         * config file.
         * @param from Source of the parsing
         * @param to Destination of the parsing
         */
        ParseResult(const std::string &source, const std::string &destination);

    public:
        /**
         * @brief type Get the type of error
         * @return Type of error or GOOD if there was no error
         */
        ErrorType type() const;

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
         * @brief setUnknownData Replace the the unknown key value pairs
         */
        void setUnknownData(
            const std::map<std::string, std::string> &unknownData);

        /**
         * @brief addUnknownData Get all the unknown key value pairs
         * @return Mapping of the unknown keys with associated data
         */
        const std::map<std::string, std::string> &getUnknownData() const;

    private:
        /// Type of the failure
        ErrorType m_result = ErrorType::UNINITIALIZED;

        /// Filename of the input file
        std::string m_inputfilename;

        /// Filename of the output file
        std::string m_outputfilename;

        /// Line number where the failure occurred (on invalid input or output
        /// file)
        size_t m_line = 0u;

        /// Description of the failure (on invalid input or output file)
        std::string m_message;

        /// All keys that contain invalid data
        std::vector<std::string> m_keys_invalidData;

        // Mapping of unknown keys and associated data
        std::map<std::string, std::string> m_unknownData;

        friend class RWConfigParser;
    };

    /**
     * @brief RWConfigParser Create a game configuration (initially invalid)
     */
    RWConfigParser() = default;

    static std::filesystem::path getDefaultConfigPath();

    std::tuple<RWConfigLayer, RWConfigParser::ParseResult> loadFile(const std::filesystem::path &path) const;

    ParseResult saveFile(const std::filesystem::path &path, const RWConfigLayer &layer) const;

    ParseResult saveFile(const std::filesystem::path &path, const RWConfigLayer &layer,
                         const std::map<std::string, std::string> &extra) const;

    /**
     * @brief layer_to_string Convert the layer to a INI string string
     * @param layer The RWConfigLayer to convert
     * @return INI string
     */
    std::tuple<std::string, RWConfigParser::ParseResult> layerToString(const RWConfigLayer &layer) const;
};

#endif // RWGAME_RWCONFIG_HPP
