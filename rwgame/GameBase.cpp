#include "GameBase.hpp"
#include "GitSHA1.h"

#include "SDL.h"

// Use first 8 chars of git hash as the build string
const std::string kBuildStr(kGitSHA1Hash, 8);
const std::string kWindowTitle = "RWGame";
constexpr int kWindowWidth = 800;
constexpr int kWindowHeight = 600;

GameBase::GameBase(Logger &inlog, int argc, char *argv[]) : log(inlog) {
    log.info("Game", "Build: " + kBuildStr);

    if (!config.isValid()) {
        throw std::runtime_error("Invalid configuration file at: " +
                                 config.getConfigFile());
    }

    size_t w = kWindowWidth, h = kWindowHeight;
    bool fullscreen = false;
    bool help = false;

    // Define and parse command line options
    namespace po = boost::program_options;
    po::options_description desc("Available options");
    desc.add_options()("help", "Show this help message")(
        "width,w", po::value<size_t>(), "Game resolution width in pixel")(
        "height,h", po::value<size_t>(), "Game resolution height in pixel")(
        "fullscreen,f", "Enable fullscreen mode")("newgame,n",
                                                  "Directly start a new game")(
        "test,t", "Starts a new game in a test location")(
        "load,l", po::value<std::string>(), "Load save file")(
        "benchmark,b", po::value<std::string>(), "Run benchmark from file");

    po::variables_map &vm = options;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    } catch (po::error &ex) {
        help = true;
        std::cout << "Error parsing arguments: " << ex.what() << std::endl;
    }

    if (help || vm.count("help")) {
        std::cout << desc;
        throw std::invalid_argument("");
    }
    if (vm.count("width")) {
        w = vm["width"].as<size_t>();
    }
    if (vm.count("height")) {
        h = vm["height"].as<size_t>();
    }
    if (vm.count("fullscreen")) {
        fullscreen = true;
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        throw std::runtime_error("Failed to initialize SDL2!");

    window.create(kWindowTitle + " [" + kBuildStr + "]", w, h, fullscreen);
}

GameBase::~GameBase() {
    SDL_Quit();

    log.info("Game", "Done cleaning up");
}
