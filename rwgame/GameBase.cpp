#include "GameBase.hpp"

//#include <rw/filesystem.hpp>

#include <iostream>

#include <SDL.h>

#include <rw/debug.hpp>

#include "GitSHA1.h"

#include <iostream>

// Use first 8 chars of git hash as the build string
const std::string kBuildStr(kGitSHA1Hash, 8);
const std::string kWindowTitle = "RWGame";
const std::string kDefaultConfigFileName = "openrw.ini";
constexpr int kWindowWidth = 800;
constexpr int kWindowHeight = 600;

GameBase::GameBase(Logger &inlog, int argc, char *argv[]) :
        log(inlog) {
    log.info("Game", "Build: " + kBuildStr);

    size_t w = kWindowWidth, h = kWindowHeight;
    rwfs::path configPath;
    bool fullscreen = false;
    bool help = false;

    // Define and parse command line options
    namespace po = boost::program_options;
    po::options_description desc_window("Window options");
    desc_window.add_options()(
        "width,w", po::value<size_t>()->value_name("WIDTH"), "Game resolution width in pixel")(
        "height,h", po::value<size_t>()->value_name("HEIGHT"), "Game resolution height in pixel")(
        "fullscreen,f", "Enable fullscreen mode");
    po::options_description desc_game("Game options");
    desc_game.add_options()(
        "newgame,n", "Start a new game")(
        "load,l", po::value<std::string>()->value_name("PATH"), "Load save file");
    po::options_description desc_devel("Developer options");
    desc_devel.add_options()(
        "test,t", "Starts a new game in a test location")(
        "benchmark,b", po::value<std::string>()->value_name("PATH"), "Run benchmark from file");
    po::options_description desc("Generic options");
    desc.add_options()(
        "config,c", po::value<rwfs::path>()->value_name("PATH"), "Path of configuration file")(
        "help", "Show this help message");
    desc.add(desc_window).add(desc_game).add(desc_devel);

    po::variables_map &vm = options;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    } catch (po::error &ex) {
        help = true;
        std::cout << "Error parsing arguments: " << ex.what() << std::endl;
    }

    if (help || (vm.count("help") != 0u)) {
        std::cout << desc;
        throw std::invalid_argument("");
    }
    if (vm.count("width") != 0u) {
        w = vm["width"].as<size_t>();
    }
    if (vm.count("height") != 0u) {
        h = vm["height"].as<size_t>();
    }
    if (vm.count("fullscreen") != 0u) {
        fullscreen = true;
    }
    if (vm.count("config") != 0u) {
        configPath = vm["config"].as<rwfs::path>();
    } else {
        configPath = GameConfig::getDefaultConfigPath() / kDefaultConfigFileName;
    }

    config.loadFile(configPath);
    if (!config.isValid()) {
        log.error("Config", "Invalid INI file at \""
            + config.getConfigPath().string() + "\".\n"
            + "Adapt the following default INI to your configuration.\n"
            + config.getDefaultINIString());
        throw std::runtime_error(config.getParseResult().what());
    }

    if (vm.count("width") == 0u) {
        w = config.getWindowWidth();
    }
    if (vm.count("height") == 0u) {
        h = config.getWindowHeight();
    }
    if (vm.count("fullscreen") == 0u) {
        fullscreen = config.getWindowFullscreen();
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        throw std::runtime_error("Failed to initialize SDL2!");
    }

    window.create(kWindowTitle + " [" + kBuildStr + "]", w, h, fullscreen);

    SET_RW_ABORT_CB([this]() {window.showCursor();},
            [this]() {window.hideCursor();});
}

GameBase::~GameBase() {
    SDL_Quit();

    log.info("Game", "Done cleaning up");
}
