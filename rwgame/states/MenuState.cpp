#include "MenuState.hpp"
#include "game.hpp"
#include "IngameState.hpp"

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
	m->addEntry(Menu::lambda("Start", [=] { StateManager::get().enter(new IngameState(game)); }));
	m->addEntry(Menu::lambda("Load Game", [=] { enterLoadMenu(); }));
	m->addEntry(Menu::lambda("Test", [=] { StateManager::get().enter(new IngameState(game, true, "test")); }));
	m->addEntry(Menu::lambda("Options", [] { RW_UNIMPLEMENTED("Options Menu"); }));
	m->addEntry(Menu::lambda("Exit", [] { StateManager::get().clear(); }));
	this->enterMenu(m);
}

void MenuState::enterLoadMenu()
{
	Menu *m = new Menu(2);
	m->offset = glm::vec2(20.f, 30.f);
	m->addEntry(Menu::lambda("Back", [=] { enterMainMenu(); }));
	auto saves = SaveGame::getAllSaveGameInfo();
	for(SaveGameInfo& save : saves) {
		if (save.valid) {
			std::stringstream ss;
			ss << save.basicState.saveTime.year << "/" << save.basicState.saveTime.month << "/" << save.basicState.saveTime.day
			   << " " << save.basicState.saveTime.hour << ":" << save.basicState.saveTime.minute << "    ";
			auto name = GameStringUtil::fromString(ss.str());
			name += save.basicState.saveName;
			m->addEntry(Menu::lambda(name, [=] {
				StateManager::get().enter(new IngameState(game, false));
				game->loadGame(save.savePath);
			}, 20.f));
		}
		else {
			m->addEntry(Menu::lambda("Corrupt", [=] { }));
		}
	}
	this->enterMenu(m);
}

void MenuState::enter()
{
	getWindow().showCursor();
}

void MenuState::exit()
{

}

void MenuState::tick(float dt)
{
	RW_UNUSED(dt);
}

void MenuState::handleEvent(const SDL_Event& e)
{
	switch(e.type) {
		case SDL_KEYUP:
			switch(e.key.keysym.sym) {
				case SDLK_ESCAPE:
					StateManager::get().exit();
				default: break;
			}
			break;
		default: break;
	}
	State::handleEvent(e);
}
