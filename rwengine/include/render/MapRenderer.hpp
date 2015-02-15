#pragma once

#include <render/OpenGLRenderer.hpp>
class GameWorld;

#define MAP_BLOCK_SIZE 63

/**
 * Utility class for rendering the world map, in the menu and radar.
 */
class MapRenderer
{
public:
	
	struct MapInfo
	{
		float scale = 1.f;
		/// World coordinate center
		glm::vec2 center;
		/// yaw of the map
		float rotation = 0.f;
		
		/// Top of the map on the screen
		glm::vec2 mapScreenBottom;
		/// Bottom of the map on the screen
		glm::vec2 mapScreenTop;
	};
	
	MapRenderer(GameWorld* world, Renderer* renderer);
	
	glm::vec2 worldToMap(const glm::vec2& coord);
	glm::vec2 mapToScreen(const glm::vec2& map, const MapInfo& mi);
	
	void draw(const MapInfo& mi);

private:
	GameWorld* world;
	Renderer* renderer;
	
	GeometryBuffer rectGeom;
	DrawBuffer rect;
	
	Renderer::ShaderProgram* rectProg;
	
	void drawBlip(const glm::vec2& map, const glm::mat4& model, const MapInfo& mi, const std::string& texture, float heading = 0.f);
};