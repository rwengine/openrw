#ifndef GAME_HPP
#define GAME_HPP

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <engine/GameObject.hpp>
#include <engine/GameWorld.hpp>


constexpr double PiOver180 = 3.1415926535897932384626433832795028/180;


bool hitWorldRay(glm::vec3& hit, glm::vec3& normal, GameObject** object = nullptr);

sf::Window& getWindow();

GameWorld* getWorld();

sf::Font& getFont();

/**
  Set view parameters.
*/

void setViewParameters(const glm::vec3& center, const glm::vec2 &angles);

#endif // GAME_HPP

