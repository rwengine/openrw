#include "pausestate.hpp"
#include "game.hpp"

PauseState::PauseState()
{
	Menu *m = new Menu(getFont());
	m->offset = glm::vec2(50.f, 100.f);
	m->addEntry(Menu::lambda("Continue", [] { StateManager::get().exit(); }));
	m->addEntry(Menu::lambda("Options", [] { std::cout << "Options" << std::endl; }));
	m->addEntry(Menu::lambda("Exit", [] { getWindow().close(); }));
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
