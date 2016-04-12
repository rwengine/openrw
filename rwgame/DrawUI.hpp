#pragma once
#include <engine/GameWorld.hpp>
#include <render/GameRenderer.hpp>

class PlayerController;

void drawHUD(ViewCamera& currentView, PlayerController* player, GameWorld* world, GameRenderer* render);

void drawOnScreenText(GameWorld* world, GameRenderer* renderer);
