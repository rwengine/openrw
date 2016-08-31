#include "RWGame.hpp"
#include "SDL.h"
#include <iostream>

int main(int argc, char* argv[])
{
	try {
		RWGame game(argc, argv);

		return game.run();
	} catch (std::runtime_error& ex) {
		// Catch runtime_error as these are fatal issues the user may want to
		// know about like corrupted files or GL initialisation failure.
		// Catching other types (out_of_range, bad_alloc) would just make
		// debugging them more difficult.

		const char* kErrorTitle = "Fatal Error";

		std::cerr << kErrorTitle << "\n" << ex.what() << std::endl;

		if (SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, kErrorTitle,
		                             ex.what(), NULL) < 0) {
			SDL_Log("Failed to show message box\n");
		}

		return -1;
	}
}
