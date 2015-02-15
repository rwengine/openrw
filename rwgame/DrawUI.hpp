#pragma once
#include <engine/GameWorld.hpp>

class PlayerController;

void drawHUD(PlayerController* player, GameWorld* world, GameRenderer* render);

void drawOnScreenText(GameWorld* world);