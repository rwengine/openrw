#include "loadingstate.hpp"
#include "RWGame.hpp"
#include <render/OpenGLRenderer.hpp>

LoadingState::LoadingState(RWGame* game)
	: State(game), next(nullptr)
{
}

void LoadingState::enter()
{
	// Load all of the files waiting to be loaded.
	auto world = getWorld();

	// Loade all of the IDEs.
	for(std::map<std::string, std::string>::iterator it = world->data->ideLocations.begin();
		it != world->data->ideLocations.end();
		++it) {
		world->defineItems(it->second);
	}

	// Load IPLs
	for(std::map<std::string, std::string>::iterator it = world->data->iplLocations.begin();
		it != world->data->iplLocations.end();
		++it) {
		world->data->loadZone(it->second);
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
	auto size = r->getRenderer()->getViewport();
	ti.size = 25.f;
	ti.screenPosition = glm::vec2( 50.f, size.y - ti.size - 50.f );
	ti.font = 2;
	r->text.renderText(ti);
}
