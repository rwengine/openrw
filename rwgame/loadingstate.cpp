#include "loadingstate.hpp"
#include "menustate.hpp"
#include "RWGame.hpp"

LoadingState::LoadingState(RWGame* game)
	: State(game)
{
}

void LoadingState::enter()
{
	// Load all of the files waiting to be loaded.
	auto world = getWorld();

	// Loade all of the IDEs.
	for(std::map<std::string, std::string>::iterator it = world->gameData.ideLocations.begin();
		it != world->gameData.ideLocations.end();
		++it) {
		world->defineItems(it->second);
	}

	// Load IPLs
	for(std::map<std::string, std::string>::iterator it = world->gameData.iplLocations.begin();
		it != world->gameData.iplLocations.end();
		++it) {
		world->gameData.loadZone(it->second);
		world->placeItems(it->second);
	}
}

void LoadingState::exit()
{

}

void LoadingState::tick(float dt)
{
	// Check to see if the GameWorld has run out of jobs
	// (i.e. it's time to open the main menu)
	if( getWorld()->_work->isEmpty() ) {
		StateManager::get().exec(new MenuState(game));
	}
}

void LoadingState::handleEvent(const sf::Event &e)
{
	State::handleEvent(e);
}

void LoadingState::draw(sf::RenderWindow &w)
{
	// Display some manner of loading screen.
	sf::Text loadingText("Loading...", game->getFont(), 28);
	loadingText.setPosition({30.f, 20.f});
	w.draw(loadingText);
}
