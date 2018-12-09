#ifndef RWGAME_GAMEINPUT_HPP
#define RWGAME_GAMEINPUT_HPP

#include <SDL_events.h>

struct GameInputState;

namespace GameInput {
void updateGameInputState(GameInputState* state, const SDL_Event& event);
}

#endif
