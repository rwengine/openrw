#include "pausestate.hpp"
#include "RWGame.hpp"
#include <ai/PlayerController.hpp>
#include <objects/CharacterObject.hpp>

PauseState::PauseState(RWGame* game)
	: State(game)
{
	Menu *m = new Menu(2);
	m->offset = glm::vec2( 200.f, 200.f );
	m->addEntry(Menu::lambda("Continue", [] { StateManager::get().exit(); }));
	m->addEntry(Menu::lambda("Options", [] { std::cout << "Options" << std::endl; }));
	m->addEntry(Menu::lambda("Exit", [] { StateManager::get().clear(); }));
	this->enterMenu(m);
}

void PauseState::enter()
{
	getWorld()->setPaused(true);

	getWindow().showCursor();
}

void PauseState::exit()
{
	getWorld()->setPaused(false);
}

void PauseState::tick(float dt)
{

}

void PauseState::draw(GameRenderer* r)
{
	MapRenderer::MapInfo map;

	auto& vp = r->getRenderer()->getViewport();

	map.worldSize = 4000.f;
	map.clipToSize = false;
	map.screenPosition = glm::vec2(vp.x/2, vp.y/2);
	map.screenSize = std::max(vp.x, vp.y);

	game->getRenderer()->map.draw(getWorld(), map);

    State::draw(r);
}

void PauseState::handleEvent(const SDL_Event& e)
{
	switch(e.type) {
		case SDL_KEYDOWN:
			switch(e.key.keysym.sym) {
				case SDLK_ESCAPE:
					StateManager::get().exit();
					break;
				default: break;
			}
			break;
		default: break;
	}
	State::handleEvent(e);
}
