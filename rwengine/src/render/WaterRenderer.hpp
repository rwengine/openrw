#pragma once

#include <rw/types.hpp>
#include <render/OpenGLRenderer.hpp>

class GameRenderer;
class GameWorld;

/**
 * Implements the rendering routines for drawing the sea water.
 */
class WaterRenderer
{
public:
  WaterRenderer(GameRenderer* renderer);
  ~WaterRenderer();

  /**
   * Creates the required data for rendering the water. Accepts
   * two arrays. waterHeights stores the real world heights which
   * are indexed into by the array tiles for each water tile.
   *
   * This data is used to create the internal stencil mask for clipping
   * the water rendering.
   */
  void setWaterTable(float* waterHeights, unsigned int nHeights, uint8_t* tiles,
                     unsigned int nTiles);

  void setDataTexture(GLuint fbBinding, GLuint dataTexture);

  /**
   * Render the water using the currently active render state
   */
  void render(GameRenderer* renderer, GameWorld* world);

private:
  Renderer::ShaderProgram* waterProg;
  Renderer::ShaderProgram* maskProg;

  DrawBuffer maskDraw;
  GeometryBuffer maskGeom;

  std::vector<int> maskSizes;

  DrawBuffer gridDraw;
  GeometryBuffer gridGeom;

  GLuint fbOutput;
  GLuint dataTexture;
};
