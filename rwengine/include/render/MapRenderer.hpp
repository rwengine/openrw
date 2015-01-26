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
		glm::vec2 viewport;
		float scale;
		glm::vec2 center;
		
		glm::vec2 mapSize;
		glm::vec2 mapPos;
	};
	
	MapRenderer(GameWorld* world, Renderer* renderer);
	
	glm::vec2 mapToScreen(const glm::vec2& map, const MapInfo& mi);
	glm::vec2 sizeOnScreen(const glm::vec2& map, const MapInfo& mi);
	
	void draw(const MapInfo& mi);

private:
	GameWorld* world;
	Renderer* renderer;
	
	GeometryBuffer rectGeom;
	DrawBuffer rect;
	
	Renderer::ShaderProgram* rectProg;
	
	void drawBlip(const glm::vec2& map, const MapInfo& mi, const std::string& texture);
};