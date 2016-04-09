#include "menustate.hpp"
#include "game.hpp"
#include "ingamestate.hpp"

#include "RWGame.hpp"
#include <engine/SaveGame.hpp>
#include <rw/defines.hpp>

MenuState::MenuState(RWGame* game)
	: State(game)
{
	enterMainMenu();
}

void MenuState::enterMainMenu()
{
	Menu *m = new Menu(2);
	m->offset = glm::vec2(200.f, 200.f);
	m->addEntry(Menu::lambda("Resume", [=] {
		StateManager::get().enter(new IngameState(game, false));
		game->loadGame("quicksave");
	}));
	m->addEntry(Menu::lambda("Start", [=] { StateManager::get().enter(new IngameState(game)); }));
	m->addEntry(Menu::lambda("Load Game", [=] { enterLoadMenu(); }));
    m->addEntry(Menu::lambda("Test", [=] { StateManager::get().enter(new IngameState(game, true, "true")); }));
	m->addEntry(Menu::lambda("Options", [] { RW_UNIMPLEMENTED("Options Menu"); }));
	m->addEntry(Menu::lambda("Exit", [=] { getWindow().close(); }));
	this->enterMenu(m);
}

void MenuState::enterLoadMenu()
{
	Menu *m = new Menu(2);
	m->offset = glm::vec2(200.f, 200.f);
	m->addEntry(Menu::lambda("Back", [=] { enterMainMenu(); }));
	auto saves = SaveGame::getAllSaveGameInfo();
	for(SaveGameInfo& save : saves) {
		m->addEntry(Menu::lambda(save.saveName, [=] {
			StateManager::get().enter(new IngameState(game, false));
			game->loadGame(save.savePath);
		}));
	}
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
