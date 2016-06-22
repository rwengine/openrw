#include "loadingstate.hpp"
#include "RWGame.hpp"
#include <render/OpenGLRenderer.hpp>

LoadingState::LoadingState(RWGame* game)
	: State(game), next(nullptr)
{
}

void LoadingState::enter()
{
	// Load Item definitions
	for( auto& def : game->getGameData()->ideLocations )
	{
		game->getGameData()->loadObjects(def.second);
	}

	game->newGame();
	getWindow().hideCursor();
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

void LoadingState::handleEvent(const SDL_Event& e)
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
	ti.baseColour = glm::u8vec3(255);
	r->text.renderText(ti);
}
