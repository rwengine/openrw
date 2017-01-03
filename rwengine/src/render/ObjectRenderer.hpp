#ifndef _RWENGINE_OBJECTRENDERER_HPP_
#define _RWENGINE_OBJECTRENDERER_HPP_

#include <engine/GameWorld.hpp>
#include <gl/DrawBuffer.hpp>
#include <glm/glm.hpp>
#include <objects/GameObject.hpp>
#include <render/OpenGLRenderer.hpp>
#include <render/ViewCamera.hpp>
#include <rw/types.hpp>

class ProjectileObject;
class PickupObject;

/**
 * @brief The ObjectRenderer class handles object -> renderer transformation
 *
 * Determines what parts of an object are within a camera frustum and exports
 * a list of things to render for the object.
 */
class ObjectRenderer {
public:
    ObjectRenderer(GameWorld* world, const ViewCamera& camera,
                   float renderAlpha, GLuint errorTexture)
        : m_world(world)
        , m_camera(camera)
        , m_renderAlpha(renderAlpha)
        , m_errorTexture(errorTexture)
        , culled(0) {
    }

    /**
     * @brief buildRenderList
     *
     * Exports rendering instructions for an object
     */
    size_t culled;
    void buildRenderList(GameObject* object, RenderList& outList);

    bool renderFrame(Clump* m, ModelFrame* f, const glm::mat4& matrix,
                     GameObject* object, float opacity, RenderList& outList);

    void renderGeometry(Clump* model, size_t g, const glm::mat4& modelMatrix,
                        float opacity, GameObject* object, RenderList& outList);

private:
    GameWorld* m_world;
    const ViewCamera& m_camera;
    float m_renderAlpha;
    GLuint m_errorTexture;

    void renderInstance(InstanceObject* instance, RenderList& outList);
    void renderCharacter(CharacterObject* pedestrian, RenderList& outList);
    void renderVehicle(VehicleObject* vehicle, RenderList& outList);
    void renderPickup(PickupObject* pickup, RenderList& outList);
    void renderCutsceneObject(CutsceneObject* cutscene, RenderList& outList);
    void renderProjectile(ProjectileObject* projectile, RenderList& outList);
};

#endif
