#ifndef _GAMERENDERER_HPP_
#define _GAMERENDERER_HPP_
#include <rw/defines.hpp>

class Logger;

#include <gl/gl_core_3_3.h>

#include <memory>
#include <vector>

#include <render/ViewCamera.hpp>

#include <render/OpenGLRenderer.hpp>
#include "MapRenderer.hpp"
#include "TextRenderer.hpp"
#include "WaterRenderer.hpp"

class Clump;
class ModelFrame;
class GameWorld;
class GameObject;

struct AreaIndicatorInfo;

/// @todo migrate to some other way of rendering each object type.
class CharacterObject;
class VehicleObject;
class InstanceObject;
class PickupObject;
class ProjectileObject;
class CutsceneObject;

class Animator;

class Renderer;

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
    Renderer* renderer;

    // Temporary variables used during rendering
    float _renderAlpha;
    GameWorld* _renderWorld;

    /** Internal non-descript VAOs */
    GLuint vao, debugVAO;

    /** Camera values passed to renderWorld() */
    ViewCamera _camera;
    ViewCamera cullingCamera;
    bool cullOverride;
    
    /** Number of culling events */
    size_t culled;

    GLuint framebufferName;
    GLuint fbTextures[2];
    GLuint fbRenderBuffers[1];
    Renderer::ShaderProgram* postProg;

    /// Texture used to replace textures missing from the data
    GLuint m_missingTexture;

public:
    GameRenderer(Logger* log, GameData* data);
    ~GameRenderer();

    /** @todo Clean up all these shader program and location variables */
    Renderer::ShaderProgram* worldProg;
    Renderer::ShaderProgram* skyProg;
    Renderer::ShaderProgram* particleProg;

    GLuint ssRectProgram;
    GLint ssRectTexture, ssRectColour, ssRectSize, ssRectOffset;

    GLuint skydomeVBO, skydomeIBO, debugVBO;
    GLuint debugTex;

    DrawBuffer skyDbuff;
    GeometryBuffer skyGbuff;

    GameData* getData() const {
        return data;
    }

    GLuint getMissingTexture() const {
        return m_missingTexture;
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
     * @brief Draws the current on screen text.
     */
    void drawOnScreenText();

    /**
     * @brief Draws a texture on the screen
     */
    void drawTexture(TextureData* texture, glm::vec4 extents);
    void drawColour(const glm::vec4& colour, glm::vec4 extents);

    /** method for rendering AI debug information */
    void renderPaths();

    /** Increases cinematic value */
    void renderLetterbox();

    void setupRender();
    void renderPostProcess();

    Renderer* getRenderer() {
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
        /// @todo what is the difference between A and B
        ZoneCylinderA,
        ZoneCylinderB,
        Arrow,
        SpecialModelCount
    };

    /**
     * @brief setSpecialModel Set model to use for each SpecialModel
     *
     * GameRenderer will take ownership of the Model* pointer
     */
    void setSpecialModel(SpecialModel usage, Clump* model) {
        specialmodels_[usage].reset(model);
    }

private:
    /// Hard-coded models to use for each of the special models
    std::unique_ptr<Clump>
        specialmodels_[SpecialModel::SpecialModelCount];
    Clump* getSpecialModel(SpecialModel usage) const {
        return specialmodels_[usage].get();
    }
};

#endif
