#include "RWGame.hpp"

std::string getGamePath()
{
	auto v = getenv(ENV_GAME_PATH_NAME);
	return v ? v : "";
}

int main(int argc, char *argv[])
{

	RWGame game( getGamePath(), argc, argv );

	return game.run();
}
