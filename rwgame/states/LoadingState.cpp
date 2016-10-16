#include "LoadingState.hpp"
#include "RWGame.hpp"

LoadingState::LoadingState(RWGame* game, std::function<void(void)> callback)
    : State(game), complete(callback) {
}

void LoadingState::enter() {
    game->newGame();
}

void LoadingState::exit() {
}

void LoadingState::tick(float dt) {
    RW_UNUSED(dt);

    // If background work is completed, do callback
    if (getWorld()->_work->isEmpty()) {
        // If we ever get back to this state it should be completed
        done();
        complete();
    }
}

bool LoadingState::shouldWorldUpdate() {
    return false;
}

void LoadingState::handleEvent(const SDL_Event& e) {
    State::handleEvent(e);
}

void LoadingState::draw(GameRenderer* r) {
    static auto kLoadingString = GameStringUtil::fromString("Loading...");
    // Display some manner of loading screen.
    TextRenderer::TextInfo ti;
    ti.text = kLoadingString;
    auto size = r->getRenderer()->getViewport();
    ti.size = 25.f;
    ti.screenPosition = glm::vec2(50.f, size.y - ti.size - 50.f);
    ti.font = 2;
    ti.baseColour = glm::u8vec3(255);
    r->text.renderText(ti);
}
