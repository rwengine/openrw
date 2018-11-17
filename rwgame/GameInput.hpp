#ifndef RWGAME_GAMEINPUT_HPP
#define RWGAME_GAMEINPUT_HPP
#include <SDL.h>
#include "engine/GameState.hpp"

namespace GameInput {
void updateGameInputState(GameInputState* state, const SDL_Event& event);
}

#endif
