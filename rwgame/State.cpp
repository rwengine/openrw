#include "State.hpp"

#include "RWGame.hpp"

// This serves as the "initial" camera position.
const ViewCamera defaultView{{-250.f, -550.f, 75.f},
                       glm::angleAxis(glm::radians(5.f),
                                      glm::vec3(0.f, 1.f, 0.f))};

State::State(RWGame *game) : game(game) {
}

void State::draw(GameRenderer& r) {
    auto& menu = getCurrentMenu();
    if (menu) {
        menu->draw(r);
    }
}

std::optional<Menu> &State::getCurrentMenu() {
    refreshMenu();
    return menu;
}

void State::handleEvent(const SDL_Event& e) {
    auto& m = getCurrentMenu();
    if (!m) return;

    switch (e.type) {
    case SDL_MOUSEBUTTONUP:
        if (e.button.button == SDL_BUTTON_LEFT)
                m->click(e.button.x, e.button.y);
            break;

        case SDL_MOUSEMOTION:
            m->hover(e.motion.x, e.motion.y);
            break;

        case SDL_KEYDOWN:
            switch (e.key.keysym.sym) {
                case SDLK_UP:
                    m->move(-1);
                    break;

                case SDLK_DOWN:
                    m->move(1);
                    break;

                case SDLK_RETURN:
                    m->activate();
                    break;
            }
    }
}

const ViewCamera& State::getCamera(float) {
    return defaultView;
}

bool State::shouldWorldUpdate() {
    return false;
}

GameWorld* State::getWorld() const {
    return game->getWorld();
}

GameWindow& State::getWindow() {
    return game->getWindow();
}

bool State::hasExited() const {
    return hasExited_;
}

void State::refreshMenu() {
    if (nextMenu) {
        menu = std::move(nextMenu);
        nextMenu = std::nullopt;
    }
}

void State::done() {
    hasExited_ = true;
}
