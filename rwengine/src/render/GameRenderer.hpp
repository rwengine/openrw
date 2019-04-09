#ifndef _RWENGINE_GAMERENDERER_HPP_
#define _RWENGINE_GAMERENDERER_HPP_

#include <cstddef>
#include <memory>

#include <gl/DrawBuffer.hpp>
#include <gl/GeometryBuffer.hpp>

#include <rw/forward.hpp>

#include <render/MapRenderer.hpp>
#include <render/OpenGLRenderer.hpp>
#include <render/TextRenderer.hpp>
#include <render/ViewCamera.hpp>
#include <render/WaterRenderer.hpp>

class TextureData;

class GameRenderer {
public:
    GameRenderer() = default;
    virtual ~GameRenderer() = default;

    virtual TextureData* findSlotTexture(const std::string& slot,
                                         const std::string& texture) const = 0;

    virtual size_t getCulledCount() = 0;

    /**
     * Renders the world using the parameters of the passed Camera.
     * Note: The camera's near and far planes are overriden by weather effects.
     *
     *  - draws all objects (instances, vehicles etc.)
     *  - draws particles
     *  - draws water surfaces
     *  - draws the skybox
     */
    virtual void renderWorld(GameWorld* world, const ViewCamera& camera,
                             float alpha) = 0;

    /**
     * @brief Draws a texture on the screen
     */
    virtual void drawTexture(TextureData* texture, glm::vec4 extents) = 0;
    virtual void drawColour(const glm::vec4& colour, glm::vec4 extents) = 0;

    virtual void setupRender() = 0;
    virtual void renderPostProcess() = 0;

    virtual void setViewport(int w, int h) = 0;

    virtual void setCullOverride(bool override, const ViewCamera& cullCamera) = 0;

    enum SpecialModel {
        ZoneCylinderA,
        ZoneCylinderB,  // same geometry as A, different UVs
        Arrow,
        SpecialModelCount
    };

    /**
     * @brief setSpecialModel Set model to use for each SpecialModel
     *
     * GameRenderer will take ownership of the Model* pointer
     */
    virtual void setSpecialModel(SpecialModel usage, const ClumpPtr& model) = 0;

    virtual void invalidate() = 0;

    virtual void useWorldProgram() = 0;
    virtual void useProgram(Renderer::ShaderProgram* p) = 0;

    virtual void drawArrays(const glm::mat4& model, DrawBuffer* draw,
                            const Renderer::DrawParameters& p) = 0;

    virtual void drawMap(GameWorld* world, const MapRenderer::MapInfo& mi) = 0;

    virtual void scaleMapHUD(const float scale) = 0;

    virtual void setWaterTable(const float* waterHeights,
                               const unsigned int nHeights,
                               const uint8_t* tiles, const unsigned int nTiles) = 0;

    virtual void renderText(const TextRenderer::TextInfo& ti,
                            bool forceColour = false) = 0;

    virtual void setFontTexture(font_t font, const std::string& textureName) = 0;

    virtual const glm::ivec2& getViewport() = 0;

    virtual void setSceneParameters(const Renderer::SceneUniformData& data) = 0;

    virtual void drawBatched(const RenderList& list) = 0;

    virtual std::unique_ptr<Renderer::ShaderProgram> createShader(
        const std::string& vert, const std::string& frag) = 0;

    virtual int getBufferCount() = 0;

    virtual int getTextureCount() = 0;

    virtual void pushDebugGroup(const std::string& title) = 0;

    virtual void setUniform(Renderer::ShaderProgram* p, const std::string& name,
                            const glm::mat4& m) = 0;
    virtual void setUniform(Renderer::ShaderProgram* p, const std::string& name,
                            const glm::vec4& v) = 0;
    virtual void setUniform(Renderer::ShaderProgram* p, const std::string& name,
                            const glm::vec3& v) = 0;
    virtual void setUniform(Renderer::ShaderProgram* p, const std::string& name,
                            const glm::vec2& v) = 0;
    virtual void setUniform(Renderer::ShaderProgram* p, const std::string& name,
                            float f) = 0;

    virtual void setUniformTexture(Renderer::ShaderProgram* p,
                                   const std::string& name, GLint tex) = 0;

    virtual const glm::mat4& get2DProjection() const = 0;

    virtual const Renderer::ProfileInfo& popDebugGroup() = 0;

    virtual void setProgramBlockBinding(Renderer::ShaderProgram* p,
                                        const std::string& name, GLint point) = 0;

    virtual int getDrawCount() = 0;

    virtual void swap() = 0;

    virtual const Renderer::SceneUniformData& getSceneData() = 0;
};

#endif
