#ifndef _RWENGINE_GAMERENDERER_HPP_
#define _RWENGINE_GAMERENDERER_HPP_

#include <cstddef>
#include <memory>

#include <glm/glm.hpp>

#include <gl/DrawBuffer.hpp>
#include <gl/GeometryBuffer.hpp>
#include <gl/gl_core_3_3.h>

#include <rw/forward.hpp>

#include <render/OpenGLRenderer.hpp>
#include <render/MapRenderer.hpp>
#include <render/TextRenderer.hpp>
#include <render/ViewCamera.hpp>
#include <render/WaterRenderer.hpp>

class Logger;
class GameData;
class GameWorld;
class TextureData;

/**
 * @brief Implements high level drawing logic and low level draw commands
 *
 * Rendering of object types is handled by drawWorld, calling the respective
 * render function for each object.
 */
class GameRenderer {
    /** Game data to use for rendering */
    GameData* data;

    /** Logger to output messages */
    Logger* logger;

    /** The low-level drawing interface to use */
    std::shared_ptr<Renderer> renderer = std::make_shared<OpenGLRenderer>();

    // Temporary variables used during rendering
    float _renderAlpha{0.f};
    GameWorld* _renderWorld = nullptr;

    /** Internal non-descript VAOs */
    GLuint vao;

    /** Camera values passed to renderWorld() */
    ViewCamera _camera;
    ViewCamera cullingCamera;
    bool cullOverride = false;

    /** Number of culling events */
    size_t culled;

    GLuint framebufferName;
    GLuint fbTextures[2];
    GLuint fbRenderBuffers[1];
    std::unique_ptr<Renderer::ShaderProgram> postProg;

    GeometryBuffer particleGeom;
    DrawBuffer particleDraw;

    GeometryBuffer ssRectGeom;
    DrawBuffer ssRectDraw;

public:
    GameRenderer(Logger* log, GameData* data);
    ~GameRenderer();

    std::unique_ptr<Renderer::ShaderProgram> worldProg;
    std::unique_ptr<Renderer::ShaderProgram> skyProg;
    std::unique_ptr<Renderer::ShaderProgram> particleProg;

    std::unique_ptr<Renderer::ShaderProgram> ssRectProg;

    GLuint skydomeIBO;

    DrawBuffer skyDbuff;
    GeometryBuffer skyGbuff;

    GameData* getData() const {
        return data;
    }

    size_t getCulledCount() {
        return culled;
    }

    /**
     * Renders the world using the parameters of the passed Camera.
     * Note: The camera's near and far planes are overriden by weather effects.
     *
     *  - draws all objects (instances, vehicles etc.)
     *  - draws particles
     *  - draws water surfaces
     *  - draws the skybox
     */
    void renderWorld(GameWorld* world, const ViewCamera& camera, float alpha);

    /**
     * Renders the effects (Particles, Lighttrails etc)
     */
    void renderEffects(GameWorld* world);

    /**
     * @brief Draws a texture on the screen
     */
    void drawTexture(TextureData* texture, glm::vec4 extents);
    void drawColour(const glm::vec4& colour, glm::vec4 extents);

    /** Render full screen splash / fade */
    void renderSplash(GameWorld* world, GLuint tex, glm::u16vec3 fc);

    /** Increases cinematic value */
    void renderLetterbox();

    void setupRender();
    void renderPostProcess();

    std::shared_ptr<Renderer> getRenderer() {
        return renderer;
    }

    void setViewport(int w, int h);

    void setCullOverride(bool override, const ViewCamera& cullCamera) {
        cullingCamera = cullCamera;
        cullOverride = override;
    }

    MapRenderer map;
    WaterRenderer water;
    TextRenderer text;

    // Profiling data
    Renderer::ProfileInfo profObjects;
    Renderer::ProfileInfo profSky;
    Renderer::ProfileInfo profWater;
    Renderer::ProfileInfo profEffects;

    enum SpecialModel {
        ZoneCylinderA,
        ZoneCylinderB,    // same geometry as A, different UVs
        Arrow,
        SpecialModelCount
    };

    /**
     * @brief setSpecialModel Set model to use for each SpecialModel
     *
     * GameRenderer will take ownership of the Model* pointer
     */
    void setSpecialModel(SpecialModel usage, const ClumpPtr& model) {
        specialmodels_[usage] = model;
    }

private:
    /// Hard-coded models to use for each of the special models
    ClumpPtr specialmodels_[SpecialModel::SpecialModelCount];
    ClumpPtr getSpecialModel(SpecialModel usage) const {
        return specialmodels_[usage];
    }

    void drawRect(const glm::vec4& colour, TextureData* texture, glm::vec4& extents);

    void renderObjects(const GameWorld *world);

    RenderList createObjectRenderList(const GameWorld *world);
};

#endif
