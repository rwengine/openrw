#ifndef GAME_HPP
#define GAME_HPP

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <objects/GameObject.hpp>
#include <engine/GameWorld.hpp>

constexpr double PiOver180 = 3.1415926535897932384626433832795028/180;

// TODO: Move all of this stuff so it's not just lying around.

bool hitWorldRay(glm::vec3& hit, glm::vec3& normal, GameObject** object = nullptr);
bool hitWorldRay(const glm::vec3& start, const glm::vec3& direction,
				 glm::vec3& hit, glm::vec3& normal, GameObject** object = nullptr);

#define GAME_TIMESTEP (1.f/30.f)
#define GAME_WINDOW_WIDTH 800
#define GAME_WINDOW_HEIGHT 600

#endif // GAME_HPP

