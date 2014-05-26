#include "menustate.hpp"
#include "game.hpp"
#include "ingamestate.hpp"

MenuState::MenuState()
{
	Menu *m = new Menu(getFont());
	m->offset = glm::vec2(50.f, 100.f);
	m->addEntry(Menu::lambda("Test", [] { StateManager::get().enter(new IngameState); }));
	m->addEntry(Menu::lambda("Options", [] { std::cout << "Options" << std::endl; }));
	m->addEntry(Menu::lambda("Exit", [] { getWindow().close(); }));
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
