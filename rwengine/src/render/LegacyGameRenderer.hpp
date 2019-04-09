#ifndef _RWENGINE_LEGACY_GAMERENDERER_HPP_
#define _RWENGINE_LEGACY_GAMERENDERER_HPP_

#include <cstddef>
#include <memory>

#include <gl/DrawBuffer.hpp>
#include <gl/GeometryBuffer.hpp>

#include <rw/forward.hpp>

#include <render/GameRenderer.hpp>
#include <render/MapRenderer.hpp>
#include <render/OpenGLRenderer.hpp>
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
class LegacyGameRenderer : public GameRenderer {
    /** Game data to use for rendering */
    GameData* data;

    /** Logger to output messages */
    Logger* logger;

    /** The low-level drawing interface to use */
    std::unique_ptr<Renderer> renderer = std::make_unique<OpenGLRenderer>();

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
    LegacyGameRenderer(Logger* log, GameData* data);
    ~LegacyGameRenderer() final;

    TextureData* findSlotTexture(const std::string& slot,
                                 const std::string& texture) const final;

    size_t getCulledCount() final {
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
    void renderWorld(GameWorld* world, const ViewCamera& camera,
                     float alpha) final;

    /**
     * @brief Draws a texture on the screen
     */
    void drawTexture(TextureData* texture, glm::vec4 extents) final;
    void drawColour(const glm::vec4& colour, glm::vec4 extents) final;

    void setupRender() final;
    void renderPostProcess() final;

    void setViewport(int w, int h) final;

    void setCullOverride(bool override, const ViewCamera& cullCamera) final {
        cullingCamera = cullCamera;
        cullOverride = override;
    }

    /**
     * @brief setSpecialModel Set model to use for each SpecialModel
     *
     * GameRenderer will take ownership of the Model* pointer
     */
    void setSpecialModel(SpecialModel usage, const ClumpPtr& model) final {
        specialmodels_[usage] = model;
    }

    void invalidate() final;

    void useWorldProgram();
    void useProgram(Renderer::ShaderProgram* p) final;

    void drawArrays(const glm::mat4& model, DrawBuffer* draw,
                    const Renderer::DrawParameters& p) final;

    void drawMap(GameWorld* world, const MapRenderer::MapInfo& mi) final;

    void scaleMapHUD(const float scale) final;

    void setWaterTable(const float* waterHeights, const unsigned int nHeights,
                       const uint8_t* tiles, const unsigned int nTiles) final;

    void renderText(const TextRenderer::TextInfo& ti,
                    bool forceColour = false) final;

    void setFontTexture(font_t font, const std::string& textureName) final;

    const glm::ivec2& getViewport() final;

    void setSceneParameters(const Renderer::SceneUniformData& data) final;

    void drawBatched(const RenderList& list) final;

    std::unique_ptr<Renderer::ShaderProgram> createShader(
        const std::string& vert, const std::string& frag) final;

    int getBufferCount() final;

    int getTextureCount() final;

    void pushDebugGroup(const std::string& title) final;

    void setUniform(Renderer::ShaderProgram* p, const std::string& name,
                    const glm::mat4& m) final;
    void setUniform(Renderer::ShaderProgram* p, const std::string& name,
                    const glm::vec4& v) final;
    void setUniform(Renderer::ShaderProgram* p, const std::string& name,
                    const glm::vec3& v) final;
    void setUniform(Renderer::ShaderProgram* p, const std::string& name,
                    const glm::vec2& v) final;
    void setUniform(Renderer::ShaderProgram* p, const std::string& name,
                    float f) final;

    void setUniformTexture(Renderer::ShaderProgram* p, const std::string& name,
                           GLint tex) final;

    const glm::mat4& get2DProjection() const final;

    const Renderer::ProfileInfo& popDebugGroup() final;

    void setProgramBlockBinding(Renderer::ShaderProgram* p,
                                const std::string& name, GLint point) final;

    int getDrawCount() final;

    void swap() final;

    const Renderer::SceneUniformData& getSceneData() final;

private:
    MapRenderer map;
    WaterRenderer water;
    TextRenderer text;

    std::unique_ptr<Renderer::ShaderProgram> worldProg;
    std::unique_ptr<Renderer::ShaderProgram> skyProg;
    std::unique_ptr<Renderer::ShaderProgram> particleProg;

    std::unique_ptr<Renderer::ShaderProgram> ssRectProg;

    /** Render full screen splash / fade */
    void renderSplash(GameWorld* world, GLuint tex, glm::u16vec3 fc);

    /**
     * Renders the effects (Particles, Lighttrails etc)
     */
    void renderEffects(GameWorld* world);

    /** Increases cinematic value */
    void renderLetterbox();

    GLuint skydomeIBO;

    DrawBuffer skyDbuff;
    GeometryBuffer skyGbuff;

    // Profiling data
    Renderer::ProfileInfo profObjects;
    Renderer::ProfileInfo profSky;
    Renderer::ProfileInfo profWater;
    Renderer::ProfileInfo profEffects;

    /// Hard-coded models to use for each of the special models
    ClumpPtr specialmodels_[SpecialModel::SpecialModelCount];
    ClumpPtr getSpecialModel(SpecialModel usage) const {
        return specialmodels_[usage];
    }

    void drawRect(const glm::vec4& colour, TextureData* texture,
                  glm::vec4& extents);

    void renderObjects(const GameWorld* world);

    RenderList createObjectRenderList(const GameWorld* world);
};

#endif
