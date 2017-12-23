#pragma once

#include <render/OpenGLRenderer.hpp>
class GameData;
class GameWorld;

#define MAP_BLOCK_SIZE 63

/**
 * Utility class for rendering the world map, in the menu and radar.
 */
class MapRenderer {
public:
    struct MapInfo {
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

    MapRenderer(std::shared_ptr<Renderer> renderer, GameData* data);

    void draw(GameWorld* world, const MapInfo& mi);

private:
    GameData* data;
    std::shared_ptr<Renderer> renderer;

    GeometryBuffer rectGeom;
    DrawBuffer rect;

    GeometryBuffer circleGeom;
    DrawBuffer circle;

    std::unique_ptr<Renderer::ShaderProgram> rectProg;

    void prepareBlip(const glm::vec2& coord, const glm::mat4& view,
                     const MapInfo& mi, const std::string& texture,
                     glm::vec4 colour, float size, float heading);
    void drawBlip(const glm::vec2& coord, const glm::mat4& view,
                  const MapInfo& mi, const std::string& texture,
                  glm::vec4 colour, float size, float heading = 0.0f);
    void drawBlip(const glm::vec2& coord, const glm::mat4& view,
                  const MapInfo& mi, glm::vec4 colour, float size);
};
