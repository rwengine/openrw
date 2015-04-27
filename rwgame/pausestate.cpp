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
	m->addEntry(Menu::lambda("Exit", [&] { getWindow().close(); }));
	this->enterMenu(m);
}

void PauseState::enter()
{
	getWorld()->setPaused(true);
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
	
	map.scale = 0.2f;
	map.mapScreenTop = glm::vec2(vp.x, vp.y);
	map.mapScreenBottom = glm::vec2(0.f, 0.f);
	
	game->getRenderer()->map.draw(getWorld(), map);

    State::draw(r);
}

void PauseState::handleEvent(const sf::Event &e)
{
	switch(e.type) {
		case sf::Event::KeyPressed:
			switch(e.key.code) {
				case sf::Keyboard::Escape:
					StateManager::get().exit();
					break;
				default: break;
			}
			break;
		default: break;
	}
	State::handleEvent(e);
}
