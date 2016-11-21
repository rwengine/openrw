#include "PauseState.hpp"
#include "RWGame.hpp"

PauseState::PauseState(RWGame* game) : State(game) {
    auto& t = game->getGameData().texts;

    auto menu = Menu::create(
        {{t.text(MenuDefaults::kResumeGameId), [&] { done(); }},
         {t.text(MenuDefaults::kOptionsId),
          [] { std::cout << "Options" << std::endl; }},
         {t.text(MenuDefaults::kQuitGameId),
          [] { StateManager::get().clear(); }}});
    menu->offset = glm::vec2(200.f, 200.f);
    enterMenu(menu);
}

void PauseState::enter() {
    getWorld()->setPaused(true);

    getWindow().showCursor();
}

void PauseState::exit() {
    getWorld()->setPaused(false);
}

void PauseState::tick(float dt) {
    RW_UNUSED(dt);
}

void PauseState::draw(GameRenderer* r) {
    MapRenderer::MapInfo map;

    auto& vp = r->getRenderer()->getViewport();

    map.worldSize = 4000.f;
    map.clipToSize = false;
    map.screenPosition = glm::vec2(vp.x / 2, vp.y / 2);
    map.screenSize = std::max(vp.x, vp.y);

    game->getRenderer().map.draw(getWorld(), map);

    State::draw(r);
}

void PauseState::handleEvent(const SDL_Event& e) {
    switch (e.type) {
        case SDL_KEYDOWN:
            switch (e.key.keysym.sym) {
                case SDLK_ESCAPE:
                    done();
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    State::handleEvent(e);
}
