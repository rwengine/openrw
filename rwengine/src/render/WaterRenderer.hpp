#ifndef _RWENGINE_WATERRENDERER_HPP_
#define _RWENGINE_WATERRENDERER_HPP_

#include <cstdint>
#include <memory>
#include <vector>

#include <gl/DrawBuffer.hpp>
#include <gl/GeometryBuffer.hpp>
#include <gl/gl_core_3_3.h>

#include <render/OpenGLRenderer.hpp>

class GameRenderer;
class GameWorld;

/**
 * Implements the rendering routines for drawing the sea water.
 */
class WaterRenderer {
public:
    WaterRenderer(GameRenderer& renderer);
    ~WaterRenderer() = default;

    /**
     * Creates the required data for rendering the water. Accepts
     * two arrays. waterHeights stores the real world heights which
     * are indexed into by the array tiles for each water tile.
     *
     * This data is used to create the internal stencil mask for clipping
     * the water rendering.
     */
    void setWaterTable(const float* waterHeights, const unsigned int nHeights,
                       const uint8_t* tiles, const unsigned int nTiles);

    void setDataTexture(GLuint fbBinding, GLuint dataTexture);

    /**
     * Render the water using the currently active render state
     */
    void render(GameRenderer& renderer, GameWorld* world);

private:
    std::unique_ptr<Renderer::ShaderProgram> waterProg = nullptr;
    std::unique_ptr<Renderer::ShaderProgram> maskProg = nullptr;

    DrawBuffer maskDraw{};
    GeometryBuffer maskGeom{};

    std::vector<int> maskSizes;

    DrawBuffer gridDraw{};
    GeometryBuffer gridGeom{};

    GLuint fbOutput{};
    GLuint dataTexture{};
};

#endif
