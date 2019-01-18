#include "RWConfig.hpp"

#include <iostream>

#include <rw/debug.hpp>

#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#ifdef RW_WINDOWS
#include <Shlobj.h>
#include <winerror.h>

#include <platform/RWWindows.hpp>
#endif

namespace po = boost::program_options;
namespace pt = boost::property_tree;

namespace {

po::options_description build_options() {
    std::array<po::options_description, RWArgumentParser::Category::COUNT_> descriptions =
    {{
         po::options_description{"Configuration options"},
         po::options_description{"Game actions"},
         po::options_description{"Input options"},
         po::options_description{"Window options"},
         po::options_description{"Developer options"},
         po::options_description{"General options"},
    }};
#define RWARG(_RW_TYPE, _RW_NAME, _RW_CATEGORY, _RW_ARGMASK, _RW_ARGMETA, _RW_HELP)                                     \
    do {                                                                                                                \
        auto option_builder = descriptions[RWArgumentParser::Category::_RW_CATEGORY].add_options();                     \
        if constexpr (std::is_same_v<bool, _RW_TYPE>) {                                                                 \
            option_builder(_RW_ARGMASK, _RW_HELP);                                                                      \
        } else {                                                                                                        \
            option_builder(_RW_ARGMASK, po::value<_RW_TYPE>()->value_name(_RW_ARGMETA), _RW_HELP);                      \
        }                                                                                                               \
    } while (0);
#define RWCONFIGARG(_RW_TYPE, _RW_NAME, _RW_DEFAULT, _RW_CONFPATH, _RW_CATEGORY, _RW_ARGMASK, _RW_ARGMETA, _RW_HELP)    \
    RWARG(_RW_TYPE, _RW_NAME, _RW_CATEGORY, _RW_ARGMASK, _RW_ARGMETA, _RW_HELP)
#define RWARG_OPT(_RW_TYPE, _RW_NAME, _RW_CATEGORY, _RW_ARGMASK, _RW_ARGMETA, _RW_HELP)                                 \
    RWARG(_RW_TYPE, _RW_NAME, _RW_CATEGORY, _RW_ARGMASK, _RW_ARGMETA, _RW_HELP)

#include "RWConfig.inc"

#undef RWARG_OPT
#undef RWCONFIGARG
#undef RWARG

    auto& description = descriptions[0];
    for (auto i = 1u; i < descriptions.size(); ++i) {
        description.add(descriptions[i]);
    }
    return description;
}

}

RWArgumentParser::RWArgumentParser() : _desc(build_options()) {
}

namespace {

constexpr std::string_view arg_mask_to_key(std::string_view v) {
    size_t maxstart = 0u, maxsize = 0u;
    size_t start = 0u;
    size_t end = 0u;
    while (start < v.size()) {
        end = v.find(",", start);
        if (end == std::string_view::npos) {
            end = v.size();
        }
        if ((end - start) > maxsize) {
            maxstart = start;
            maxsize = end - start;
        }
        start = end + 1;
    }
    return v.substr(maxstart, maxsize);
}

template <typename T>
std::optional<T> vm_get_opt_argmask(const po::variables_map& vm, const std::string_view& argmask) {
    const std::string key{arg_mask_to_key(argmask)};
    auto it = vm.find(key);
    if (it != vm.end()) {
        return it->second.as<T>();
    }
    return std::nullopt;
}

template <>
std::optional<bool> vm_get_opt_argmask<bool>(const po::variables_map& vm, const std::string_view& argmask) {
    const std::string key{arg_mask_to_key(argmask)};
    auto it = vm.find(key);
    if (it != vm.end()) {
        return true;
    }
    return std::nullopt;
}

bool vm_get_argmask(const po::variables_map& vm, const std::string_view& argmask) {
    auto val = vm_get_opt_argmask<bool>(vm, argmask);
    return val.has_value() && *val;
}
}

std::optional<RWArgConfigLayer> RWArgumentParser::parseArguments(int argc, const char* argv[]) const {
    po::variables_map vm;
    try {
        if (argc != 0) {
            po::command_line_parser parser(argc, argv);
            parser.options(_desc);
            parser.positional(po::positional_options_description{});
            parser.style(po::command_line_style::unix_style & (~po::command_line_style::allow_guessing));
            po::store(parser.run(), vm);
        }
        po::notify(vm);
    } catch (po::error& ex) {
        std::cerr << "Error parsing arguments: " << ex.what() << std::endl;
        return std::nullopt;
    } catch (boost::exception& ex) {
        std::cerr << "A boost::exception object was thrown (bug in Boost.Program_options?).\n";
        std::cerr << "unknown error\n";
        return std::nullopt;
    }
    RWArgConfigLayer layer;

#define RWARG(_RW_TYPE, _RW_NAME, _RW_CATEGORY, _RW_ARGMASK, _RW_ARGMETA, _RW_HELP)                                     \
    do {                                                                                                                \
        layer._RW_NAME = vm_get_argmask(vm, _RW_ARGMASK);                                                               \
    } while (0);
#define RWCONFIGARG(_RW_TYPE, _RW_NAME, _RW_DEFAULT, _RW_CONFPATH, _RW_CATEGORY, _RW_ARGMASK, _RW_ARGMETA, _RW_HELP)    \
    RWARG_OPT(_RW_TYPE, _RW_NAME, _RW_CATEGORY, _RW_ARGMASK, _RW_ARGMETA, _RW_HELP)
#define RWARG_OPT(_RW_TYPE, _RW_NAME, _RW_CATEGORY, _RW_ARGMASK, _RW_ARGMETA, _RW_HELP)                                 \
    do {                                                                                                                \
        layer._RW_NAME = vm_get_opt_argmask<_RW_TYPE>(vm, _RW_ARGMASK);                                                 \
    } while (0);

#include "RWConfig.inc"

#undef RWARG_OPT
#undef RWCONFIGARG
#undef RWARG

    if (layer.noconfig && layer.configPath.has_value()) {
        std::cerr << "Cannot set config path and noconfig at the same time.\n";
        return std::nullopt;
    }

    return layer;
}

std::ostream& RWArgumentParser::printHelp(std::ostream &os) const {
    return os << _desc;
}

RWConfigLayer buildDefaultConfigLayer() {
    RWConfigLayer layer;

#define RWARG(...)
#define RWCONFIGARG(_RW_TYPE, _RW_NAME, _RW_DEFAULT, _RW_CONFPATH, _RW_CATEGORY, _RW_ARGMASK, _RW_ARGMETA, _RW_HELP)    \
    layer._RW_NAME = _RW_DEFAULT;
#define RWARG_OPT(...)
#include "RWConfig.inc"
#undef RWARG_OPT
#undef RWCONFIGARG
#undef RWARG

    return layer;
}

static constexpr auto kConfigDirectoryName = "OpenRW";

rwfs::path RWConfigParser::getDefaultConfigPath() {
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

namespace {

void flatten_ptree_recursive(std::map<std::string, std::string> &map, const std::string &subkey, const pt::ptree &ptree) {
    for (const auto &[name, content] : ptree) {
        auto key = subkey + "." + name;
        map[key] = content.data();
        flatten_ptree_recursive(map, subkey + "." + name, content);
    }
}
std::map<std::string, std::string> flatten_ptree(const pt::ptree &ptree) {
    std::map<std::string, std::string> result;
    for (const auto &[name, content] : ptree) {
        flatten_ptree_recursive(result, name, content);
    }
    return result;
}

std::string stripComments(const std::string &str) {
    auto s = std::string(str, 0, str.find_first_of(";#"));
    return s.erase(s.find_last_not_of(" \n\r\t") + 1);
}

template <typename T>
struct Translator {
};

template <>
struct Translator<std::string> {
    using internal_type = std::string;
    using external_type = std::string;
    boost::optional<external_type> get_value(const internal_type &str) {
        return stripComments(str);
    }
    boost::optional<internal_type> put_value(const external_type &str) {
        return str;
    }
};

template <>
struct Translator<bool> {
    using internal_type = std::string;
    using external_type = bool;
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

template <>
struct Translator<int> {
    using internal_type = std::string;
    using external_type = int;
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

template <>
struct Translator<float> {
    using internal_type = std::string;
    using external_type = float;
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

class TreeParser {
    template <typename T>
    using TGetFunction = std::function<const std::optional<T>(const RWConfigLayer &)>;
    template <typename T>
    using TSetFunction = std::function<void(RWConfigLayer &, const std::optional<T> &)>;
public:
    TreeParser() = default;
    template <typename T, typename TString, typename TGetFunc = TGetFunction<T>, typename TSetFunc = TSetFunction<T>>
    void add_option(TString &&key, TGetFunc &&getFunc, TSetFunc &&setFunc) {
        _itemParsers.emplace_back(
            std::make_unique<TreeItemParserImpl<T>>(
                std::forward<TString>(key), std::forward<TGetFunc>(getFunc), std::forward<TSetFunc>(setFunc)));
    }
    RWConfigLayer to_layer(const pt::ptree &ptree, RWConfigParser::ParseResult &parseResult) const {
        RWConfigLayer layer;
        auto flattened_ptree = flatten_ptree(ptree);
        for (const auto &itemParser : _itemParsers) {
            try {
                itemParser->to_layer(layer, ptree);
                flattened_ptree.erase(itemParser->key());
            } catch (pt::ptree_bad_path &) {
                // bad path -> not found -> no-op / std::nullopt
            } catch (pt::ptree_bad_data &) {
                parseResult.failInvalidData(itemParser->key());
            }
        }
        parseResult.setUnknownData(flattened_ptree);
        return layer;
    }

pt::ptree to_ptree(const RWConfigLayer &layer, RWConfigParser::ParseResult &parseResult) const {
        pt::ptree ptree;
        for (const auto &itemParser : _itemParsers) {
            try {
                itemParser->to_ptree(ptree, layer);
            } catch (pt::ptree_bad_path &) {
                // bad path -> path has wrong format
                parseResult.failInvalidData(itemParser->key());
            } catch (pt::ptree_bad_data &) {
                parseResult.failInvalidData(itemParser->key());
            }
        }
        return ptree;
    }
private:
    class TreeItemParser {
    protected:
        const std::string _key;
        template <typename TString>
        TreeItemParser(TString &&key) : _key(key) {}
    public:
        virtual ~TreeItemParser() = default;
        virtual void to_layer(RWConfigLayer &layer, const pt::ptree &ptree) const = 0;
        virtual void to_ptree(pt::ptree &ptree, const RWConfigLayer &layer) const = 0;
        const std::string &key() const {
            return _key;
        }
    };
    template <typename T>
    class TreeItemParserImpl : public TreeItemParser {
    public:
        template <typename TString>
        TreeItemParserImpl(TString &&key, TGetFunction<T> &&getFunction, TSetFunction<T> &&setFunction)
            : TreeItemParser(std::forward<TString>(key))
            , _getFunction(getFunction)
            , _setFunction(setFunction) {
        }
        ~TreeItemParserImpl() override = default;
        TGetFunction<T> _getFunction;
        TSetFunction<T> _setFunction;
        void to_layer(RWConfigLayer &layer, const pt::ptree &ptree) const override {
            Translator<T> translator{};
            auto value = ptree.get<T>(_key, translator);
            auto stl_optional = std::optional<T>(value);
            _setFunction(layer, stl_optional);
        }
        void to_ptree(pt::ptree &ptree, const RWConfigLayer &layer) const override {
            Translator<T> translator{};
            const auto stl_optional = _getFunction(layer);
            if (stl_optional.has_value()) {
                T value = *stl_optional;
                ptree.put(_key, value, translator);
            }
        }
    };
    std::vector<std::unique_ptr<TreeItemParser>> _itemParsers;
};

TreeParser buildTreeParser() {
    TreeParser treeParser;
#define RWCONFIGARG(_RW_TYPE, _RW_NAME, _RW_DEFAULT, _RW_CONFPATH, _RW_CATEGORY, _RW_ARGMASK, _RW_ARGMETA, _RW_HELP)        \
    treeParser.add_option<_RW_TYPE>(                                                                                        \
        _RW_CONFPATH,                                                                                                       \
        [](const RWConfigLayer &layer) {return layer._RW_NAME;},                                                            \
        [](RWConfigLayer &layer, const std::optional<_RW_TYPE> &value) {layer._RW_NAME = value;}                            \
    );
#define RWARG(...)
#define RWARG_OPT(...)

#include "RWConfig.inc"

#undef RWCONFIGARG
#undef RWARG
#undef RWARG_OPT
    return treeParser;
}

}

std::tuple<RWConfigLayer, RWConfigParser::ParseResult> RWConfigParser::loadFile(const rwfs::path &path) const {
    ParseResult parseResult(path.string(), "<internal>");

    auto treeParser = buildTreeParser();

    RWConfigLayer layer;
    try {
        pt::ptree ptree;
        pt::read_ini(path.string(), ptree);
        layer = treeParser.to_layer(ptree, parseResult);
    } catch (pt::ini_parser_error &e) {
        // Catches illegal input files (nonsensical input, duplicate keys)
        parseResult.failInputFile(e.line(), e.message());
        RW_MESSAGE(e.what());
    }

    if (parseResult.type() == ParseResult::UNINITIALIZED) {
        parseResult.markGood();
    }

    return std::make_tuple(layer, parseResult);
}

RWConfigParser::ParseResult RWConfigParser::saveFile(const rwfs::path &path, const RWConfigLayer &layer, const std::map<std::string, std::string> &extra) const {
    ParseResult parseResult("<internal>", path.string());

    auto treeParser = buildTreeParser();

    try {
        auto ptree = treeParser.to_ptree(layer, parseResult);
        for (const auto &[key, value] : extra) {
            if (ptree.count(key) != 0u) {
                parseResult.failInvalidData(key);
            }
            ptree.put(key, value);
        }
        pt::write_ini(path.string(), ptree);
    } catch (pt::ini_parser_error &e) {
        parseResult.failOutputFile(e.line(), e.message());
    }

    if (parseResult.type() == ParseResult::UNINITIALIZED) {
        parseResult.markGood();
    }

    return parseResult;
}

RWConfigParser::ParseResult RWConfigParser::saveFile(const rwfs::path &path, const RWConfigLayer &layer) const {
    ParseResult parseResult("<internal>", path.string());

    auto treeParser = buildTreeParser();

    try {
        auto ptree = treeParser.to_ptree(layer, parseResult);
        pt::write_ini(path.string(), ptree);
    } catch (pt::ini_parser_error &e) {
        parseResult.failOutputFile(e.line(), e.message());
    }

    if (parseResult.type() == ParseResult::UNINITIALIZED) {
        parseResult.markGood();
    }

    return parseResult;
}

std::tuple<std::string, RWConfigParser::ParseResult> RWConfigParser::layerToString(const RWConfigLayer &layer) const {
    ParseResult parseResult("<internal>", "<string>");

    auto treeParser = buildTreeParser();

    std::string result;
    try {
        auto ptree = treeParser.to_ptree(layer, parseResult);
        std::ostringstream oss;
        pt::write_ini(oss, ptree);
        result = oss.str();
    } catch (pt::ini_parser_error &e) {
        parseResult.failOutputFile(e.line(), e.message());
    }

    if (parseResult.type() == ParseResult::UNINITIALIZED) {
        parseResult.markGood();
    }

    return std::make_tuple(result, parseResult);
}

RWConfigParser::ParseResult::ParseResult(const std::string &source, const std::string &destination)
    : m_result(ErrorType::UNINITIALIZED)
    , m_inputfilename(source)
    , m_outputfilename(destination) {
}

RWConfigParser::ParseResult::ErrorType RWConfigParser::ParseResult::type() const {
    return this->m_result;
}

bool RWConfigParser::ParseResult::isValid() const {
    return this->type() == ErrorType::GOOD;
}

void RWConfigParser::ParseResult::failInputFile(size_t line,
                                            const std::string &message) {
    this->m_result = ParseResult::ErrorType::INVALIDINPUTFILE;
    this->m_line = line;
    this->m_message = message;
}

void RWConfigParser::ParseResult::markGood() {
    this->m_result = ParseResult::ErrorType::GOOD;
}

void RWConfigParser::ParseResult::failInvalidData(const std::string &key) {
    this->m_result = ParseResult::ErrorType::INVALIDCONTENT;
    this->m_keys_invalidData.push_back(key);
}

void RWConfigParser::ParseResult::failOutputFile(size_t line,
                                             const std::string &message) {
    this->m_result = ParseResult::ErrorType::INVALIDOUTPUTFILE;
    this->m_line = line;
    this->m_message = message;
}

const std::vector<std::string> &RWConfigParser::ParseResult::getKeysInvalidData()
    const {
    return this->m_keys_invalidData;
}

std::string RWConfigParser::ParseResult::what() const {
    std::ostringstream oss;
    switch (this->m_result) {
        case ErrorType::UNINITIALIZED:
            oss << "Parsing was skipped or did not finish.";
            break;
        case ErrorType::GOOD:
            oss << "Parsing completed without errors.";
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
            RW_UNUSED(value);
            oss << "\n - " << key;
        }
    }
    return oss.str();
}

const std::map<std::string, std::string>
    &RWConfigParser::ParseResult::getUnknownData() const {
    return this->m_unknownData;
}

void RWConfigParser::ParseResult::setUnknownData(
    const std::map<std::string, std::string> &unknownData) {
    this->m_unknownData = unknownData;
}
