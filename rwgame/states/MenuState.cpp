#include "MenuState.hpp"
#include "IngameState.hpp"
#include "game.hpp"

#include <engine/SaveGame.hpp>
#include <rw/debug.hpp>
#include "RWGame.hpp"

MenuState::MenuState(RWGame* game) : State(game) {
    enterMainMenu();
}

void MenuState::enterMainMenu() {
    auto& t = game->getGameData().texts;

    auto menu = Menu::create(
        {{t.text(MenuDefaults::kStartGameId),
          [=] { StateManager::get().enter<IngameState>(game); }},
         {t.text(MenuDefaults::kLoadGameId), [=] { enterLoadMenu(); }},
         {t.text(MenuDefaults::kDebugId),
          [=] { StateManager::get().enter<IngameState>(game, true, "test"); }},
         {t.text(MenuDefaults::kOptionsId),
          [] { RW_UNIMPLEMENTED("Options Menu"); }},
         {t.text(MenuDefaults::kQuitGameId),
          [] { StateManager::get().clear(); }}});
    menu->offset = glm::vec2(200.f, 200.f);

    enterMenu(menu);
}

void MenuState::enterLoadMenu() {
    auto menu = Menu::create({{"BACK", [=] { enterMainMenu(); }}});

    auto saves = SaveGame::getAllSaveGameInfo();
    if (saves.has_value()) {
        for (SaveGameInfo& save : *saves) {
            if (save.valid) {
                std::stringstream ss;
                ss << save.basicState.saveTime.year << " "
                   << save.basicState.saveTime.month << " "
                   << save.basicState.saveTime.day << " "
                   << save.basicState.saveTime.hour << ":"
                   << save.basicState.saveTime.minute << "    ";
                auto name = GameStringUtil::fromString(ss.str(), FONT_ARIAL);
                name += save.basicState.saveName;
                auto loadsave = [=] {
                    StateManager::get().enter<IngameState>(game, false);
                    game->loadGame(save.savePath);
                };
                menu->lambda(name, loadsave);
            } else {
                menu->lambda("CORRUPT", [=] {});
            }
        }
    }
    menu->offset = glm::vec2(20.f, 30.f);

    enterMenu(menu);
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
                    done();
                default:
                    break;
            }
            break;
        default:
            break;
    }
    State::handleEvent(e);
}
