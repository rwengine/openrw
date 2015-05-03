#include "menustate.hpp"
#include "game.hpp"
#include "ingamestate.hpp"

#include "RWGame.hpp"

MenuState::MenuState(RWGame* game)
	: State(game)
{
	Menu *m = new Menu(2);
	m->offset = glm::vec2(200.f, 200.f);
	m->addEntry(Menu::lambda("Start", [=] { StateManager::get().enter(new IngameState(game)); }));
	m->addEntry(Menu::lambda("Resume", [=] {
		StateManager::get().enter(new IngameState(game, false));
		game->loadGame("quicksave");
	}));
	m->addEntry(Menu::lambda("Test", [=] { StateManager::get().enter(new IngameState(game, true, true)); }));
	m->addEntry(Menu::lambda("Options", [] { std::cout << "Options" << std::endl; }));
	m->addEntry(Menu::lambda("Exit", [=] { getWindow().close(); }));
	this->enterMenu(m);
}

void MenuState::enter()
{

}

void MenuState::exit()
{

}

void MenuState::tick(float dt)
{

}

void MenuState::handleEvent(const sf::Event &e)
{
	switch(e.type) {
		case sf::Event::KeyPressed:
			switch(e.key.code) {
				case sf::Keyboard::Escape:
					StateManager::get().exit();
				default: break;
			}
			break;
		default: break;
	}
	State::handleEvent(e);
}
