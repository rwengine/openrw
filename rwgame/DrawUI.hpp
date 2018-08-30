#ifndef _RWGAME_DRAWUI_HPP_
#define _RWGAME_DRAWUI_HPP_
#include <engine/GameWorld.hpp>
#include <render/GameRenderer.hpp>

class PlayerController;

void drawHUD(ViewCamera& currentView, PlayerController* player,
             GameWorld* world, GameRenderer* render);

void drawOnScreenText(GameWorld* world, GameRenderer* renderer);

#endif
