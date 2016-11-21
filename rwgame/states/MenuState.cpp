#include "MenuState.hpp"
#include "IngameState.hpp"
#include "game.hpp"

#include <engine/SaveGame.hpp>
#include <rw/defines.hpp>
#include "RWGame.hpp"

MenuState::MenuState(RWGame* game) : State(game) {
    enterMainMenu();
}

void MenuState::enterMainMenu() {
    auto data = game->getGameData();
    auto& t = data->texts;

    Menu* m = new Menu;
    m->offset = glm::vec2(200.f, 200.f);
    m->addEntry(Menu::lambda(t.text(MenuDefaults::kStartGameId), [=] {
        StateManager::get().enter(new IngameState(game));
    }));
    m->addEntry(Menu::lambda(t.text(MenuDefaults::kLoadGameId),
                             [=] { enterLoadMenu(); }));
    m->addEntry(Menu::lambda(t.text(MenuDefaults::kDebugId), [=] {
        StateManager::get().enter(new IngameState(game, true, "test"));
    }));
    m->addEntry(Menu::lambda(t.text(MenuDefaults::kOptionsId),
                             [] { RW_UNIMPLEMENTED("Options Menu"); }));
    m->addEntry(Menu::lambda(t.text(MenuDefaults::kQuitGameId),
                             [] { StateManager::get().clear(); }));
    this->enterMenu(m);
}

void MenuState::enterLoadMenu() {
    Menu* m = new Menu;
    m->offset = glm::vec2(20.f, 30.f);
    m->addEntry(Menu::lambda("BACK", [=] { enterMainMenu(); }));
    auto saves = SaveGame::getAllSaveGameInfo();
    for (SaveGameInfo& save : saves) {
        if (save.valid) {
            std::stringstream ss;
            ss << save.basicState.saveTime.year << " "
               << save.basicState.saveTime.month << " "
               << save.basicState.saveTime.day << " "
               << save.basicState.saveTime.hour << ":"
               << save.basicState.saveTime.minute << "    ";
            auto name = GameStringUtil::fromString(ss.str());
            name += save.basicState.saveName;
            m->addEntry(Menu::lambda(name,
                                     [=] {
                                         StateManager::get().enter(
                                             new IngameState(game, false));
                                         game->loadGame(save.savePath);
                                     },
                                     20.f));
        } else {
            m->addEntry(Menu::lambda("CORRUPT", [=] {}));
        }
    }
    this->enterMenu(m);
}

void MenuState::enter() {
    getWindow().showCursor();
}

void MenuState::exit() {
}

void MenuState::tick(float dt) {
    RW_UNUSED(dt);
}

void MenuState::handleEvent(const SDL_Event& e) {
    switch (e.type) {
        case SDL_KEYUP:
            switch (e.key.keysym.sym) {
                case SDLK_ESCAPE:
                    StateManager::get().exit();
                default:
                    break;
            }
            break;
        default:
            break;
    }
    State::handleEvent(e);
}
