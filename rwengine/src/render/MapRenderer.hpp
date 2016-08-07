#pragma once

#include <render/OpenGLRenderer.hpp>
class GameData;
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
		/// World coordinate center
		glm::vec2 worldCenter;
		/// World units to fit on the map
		float worldSize;

		/// yaw of the map
		float rotation = 0.f;

		glm::vec2 screenPosition;
		float screenSize;
		/// Make the map circular, or don't.
		bool clipToSize = true;
	};
	
	MapRenderer(Renderer* renderer, GameData* data);
	
	void draw(GameWorld* world, const MapInfo& mi);

private:
	GameData* data;
	Renderer* renderer;
	
	GeometryBuffer rectGeom;
	DrawBuffer rect;

	GeometryBuffer circleGeom;
	DrawBuffer circle;
	
	Renderer::ShaderProgram* rectProg;
	
	void drawBlip(const glm::vec2& map, const glm::mat4& view, const MapInfo& mi, const std::string& texture, float heading = 0.f, float size = 18.f);
};
