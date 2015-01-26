#include "pausestate.hpp"
#include "RWGame.hpp"
#include <ai/PlayerController.hpp>
#include <objects/CharacterObject.hpp>

PauseState::PauseState(RWGame* game)
	: State(game)
{
	Menu *m = new Menu(game->getFont());
	m->offset = glm::vec2(50.f, 100.f);
	m->addEntry(Menu::lambda("Continue", [] { StateManager::get().exit(); }));
	m->addEntry(Menu::lambda("Options", [] { std::cout << "Options" << std::endl; }));
	m->addEntry(Menu::lambda("Exit", [&] { getWindow().close(); }));
	this->enterMenu(m);
}

void PauseState::enter()
{

}

void PauseState::exit()
{

}

void PauseState::tick(float dt)
{

}

void PauseState::draw(sf::RenderWindow& w)
{
	MapRenderer::MapInfo map;
	
	map.scale = 0.25f;
	map.viewport = glm::vec2(w.getSize().x, w.getSize().y);
	map.mapSize = map.viewport;
	map.mapPos = map.viewport / 2.f;
	
	if( getWorld()->state.player )
	{
		map.center = glm::vec2(getWorld()->state.player->getCharacter()->getPosition());
	}
	getWorld()->renderer.map.draw(map);

    State::draw(w);
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
