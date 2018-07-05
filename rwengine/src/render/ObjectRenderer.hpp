#ifndef _RWENGINE_OBJECTRENDERER_HPP_
#define _RWENGINE_OBJECTRENDERER_HPP_

#include <cstddef>

#include <gl/gl_core_3_3.h>

//#include <engine/GameWorld.hpp>
//#include <gl/DrawBuffer.hpp>
#include <glm/glm.hpp>
//#include <objects/GameObject.hpp>
#include <render/OpenGLRenderer.hpp>
//#include <render/ViewCamera.hpp>
//#include <rw/types.hpp>

class Atomic;
class CharacterObject;
class Clump;
class CutsceneObject;
class GameObject;
class GameWorld;
class InstanceObject;
class PickupObject;
class ProjectileObject;
class VehicleObject;
class ViewCamera;
struct Geometry;

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
        , m_errorTexture(errorTexture) {
    }

    /**
     * @brief buildRenderList
     *
     * Exports rendering instructions for an object
     */
    size_t culled = 0;
    void buildRenderList(GameObject* object, RenderList& outList);

    void renderGeometry(Geometry* geom, const glm::mat4& modelMatrix,
                        GameObject* object, RenderList& outList);

    /**
     * @brief renderAtomic renders an atomic with the given worldtransform
     * @param model
     * @param atomic
     * @param worldtransform
     * @param object
     */
    void renderAtomic(Atomic* atomic, const glm::mat4& worldtransform, GameObject* object, RenderList& render);

    /**
     * @brief renderClump Renders all visible atomics in the clump
     * @param model
     * @param worldtransform
     * @param render
     */
    void renderClump(Clump* model, const glm::mat4& worldtransform, GameObject* object, RenderList& render);

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
