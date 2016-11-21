#ifndef RWGAME_GAMEINPUT_HPP
#define RWGAME_GAMEINPUT_HPP
#include "engine/GameState.hpp"
#include "SDL2/SDL.h"

namespace GameInput {
void updateGameInputState(GameInputState* state, const SDL_Event& event);
}

#endif
