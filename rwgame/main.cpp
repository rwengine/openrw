#include <iostream>
#include "RWGame.hpp"
#include "SDL.h"

#include <core/Logger.hpp>

int main(int argc, char* argv[]) {
    // Initialise Logging before anything else happens
    StdOutReceiver logstdout;
    Logger logger({ &logstdout });

    try {
        RWGame game(logger, argc, argv);

        return game.run();
    } catch (std::invalid_argument& ex) {
        // This exception is thrown when either an invalid command line option
        // or a --help is found. The RWGame constructor prints a usage message
        // in this case and then throws this exception.
        return -2;
    } catch (std::runtime_error& ex) {
        // Catch runtime_error as these are fatal issues the user may want to
        // know about like corrupted files or GL initialisation failure.
        // Catching other types (out_of_range, bad_alloc) would just make
        // debugging them more difficult.

        const char* kErrorTitle = "Fatal Error";

        logger.error("exception", ex.what());

        if (SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, kErrorTitle,
                                     ex.what(), NULL) < 0) {
            SDL_Log("Failed to show message box\n");
        }

        SDL_Quit();

        return -1;
    }
}
