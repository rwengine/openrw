#define SDL_MAIN_HANDLED

#include <iostream>

#include "RWGame.hpp"
#include <SDL.h>

#include <core/Logger.hpp>

#include "RWConfig.hpp"

int main(int argc, const char* argv[]) {
    // Initialise Logging before anything else happens
    StdOutReceiver logstdout;
    Logger logger({ &logstdout });

    RWArgumentParser argParser;
    auto argLayerOpt = argParser.parseArguments(argc, argv);
    if (!argLayerOpt.has_value()) {
        argParser.printHelp(std::cerr);
        return 1;
    }
    if (argLayerOpt->help) {
        argParser.printHelp(std::cout);
        return 0;
    }

    SDL_SetMainReady();

    try {
        RWGame game(logger, argLayerOpt);

        return game.run();
    } catch (std::runtime_error& ex) {
        // Catch runtime_error as these are fatal issues the user may want to
        // know about like corrupted files or GL initialisation failure.
        // Catching other types (out_of_range, bad_alloc) would just make
        // debugging them more difficult.

        static constexpr char const* kErrorTitle = "Fatal Error";

        logger.error("exception", ex.what());

        if (SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, kErrorTitle,
                                     ex.what(), nullptr) < 0) {
            SDL_Log("Failed to show message box\n");
        }

        SDL_Quit();

        return -1;
    }
}
