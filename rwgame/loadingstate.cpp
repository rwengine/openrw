#include "loadingstate.hpp"
#include "RWGame.hpp"

LoadingState::LoadingState(RWGame* game)
	: State(game), next(nullptr)
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
	// If background work is completed, switch to the next state
	if( getWorld()->_work->isEmpty() ) {
		StateManager::get().exec(next);
	}
}

bool LoadingState::shouldWorldUpdate()
{
	return false;
}

void LoadingState::setNextState(State* nextState)
{
	next = nextState;
}

void LoadingState::handleEvent(const sf::Event &e)
{
	State::handleEvent(e);
}

void LoadingState::draw(GameRenderer* r)
{
	// Display some manner of loading screen.
	TextRenderer::TextInfo ti;
	ti.text = "Loading...";
	ti.screenPosition = glm::vec2( 50.f, 50.f );
	ti.size = 10.f;
	ti.font = 2;
	r->text.renderText(ti);
}
