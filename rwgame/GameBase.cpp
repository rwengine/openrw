#include "GameBase.hpp"

#include <core/Logger.hpp>
#include <rw/debug.hpp>
#include "GitSHA1.h"

#include <SDL.h>

#include <iostream>

// Use first 8 chars of git hash as the build string
const std::string kBuildStr(kGitSHA1Hash, 8);
const std::string kWindowTitle = "RWGame";

GameBase::GameBase(Logger &inlog, const std::optional<RWArgConfigLayer> &args) :
        log(inlog),
        config(buildConfig(args)) {
    log.info("Game", "Build: " + kBuildStr);

    bool fullscreen = config.fullscreen();
    size_t w = config.width(), h = config.height();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
        throw std::runtime_error("Failed to initialize SDL2!");

    window.create(kWindowTitle + " [" + kBuildStr + "]", w, h, fullscreen);

    SET_RW_ABORT_CB([this]() {window.showCursor();},
            [this]() {window.hideCursor();});
}

RWConfig GameBase::buildConfig(const std::optional<RWArgConfigLayer> &args) {
    RWConfig config;
    if (args.has_value()) {
        config.setLayer(RWConfig::LAYER_ARGUMENT, *args);
    }
    auto defaultLayer = buildDefaultConfigLayer();
    config.setLayer(RWConfig::LAYER_DEFAULT, defaultLayer);

    std::filesystem::path configPath;
    if (args.has_value() && args->configPath.has_value()) {
        configPath = *args->configPath;
    } else {
        configPath = RWConfigParser::getDefaultConfigPath() / "openrw.ini";
    }

    if ((!args) || (args && !args->noconfig)) {
        RWConfigParser configParser{};
        auto [configLayer, parseResult] = configParser.loadFile(configPath);

        if (!parseResult.isValid()) {
            log.error("Config", "Could not read configuation file at " + configPath.string());
            throw std::runtime_error(parseResult.what());
        }
        config.unknown = parseResult.getUnknownData();
        config.setLayer(RWConfig::LAYER_CONFIGFILE, configLayer);
    }

    auto missingKeys = config.missingKeys();
    if (!missingKeys.empty()) {
        std::ostringstream oss;
        oss << "Configuration is incomplete. The following configuration parameters are missing:";
        for (const auto &missingKey : missingKeys) {
            oss << "\n- " << missingKey << '\n';
        }
        defaultLayer.gamedataPath = "/path/to/gta3/data";
        RWConfigParser configParser{};
        auto [default_ini_string, parseResult] = configParser.layerToString(defaultLayer);
        log.error("Config", "Configuration is incomplete. INI file at \"" + configPath.string() + "\"");
        if (parseResult.isValid()) {
            log.error("Config", "Adapt the following default INI to your configuration.");
            log.error("Config", default_ini_string);
        } else {
            log.error("Config", "Default INI creation failed.");
            oss << "\n On top, an internal error occured while creating the default INI string.";
        }

        throw std::runtime_error(oss.str());
    }
    return config;
}

GameBase::~GameBase() {
    window.close();

    SDL_Quit();

    log.info("Game", "Done cleaning up");
}
