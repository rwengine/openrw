#include <data/Clump.hpp>
#include <data/CutsceneData.hpp>
#include <data/Skeleton.hpp>
#include <engine/GameData.hpp>
#include <engine/GameState.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <render/ObjectRenderer.hpp>

// Objects that we know how to turn into renderlist entries
#include <objects/CharacterObject.hpp>
#include <objects/CutsceneObject.hpp>
#include <objects/InstanceObject.hpp>
#include <objects/PickupObject.hpp>
#include <objects/ProjectileObject.hpp>
#include <objects/VehicleObject.hpp>
#ifdef RW_WINDOWS
#include <rw_mingw.hpp>
#endif

constexpr float kDrawDistanceFactor = 1.0f;
constexpr float kWorldDrawDistanceFactor = kDrawDistanceFactor;
#if 0  // There's no distance based culling for these types of objects yet
constexpr float kVehicleDrawDistanceFactor = kDrawDistanceFactor;
constexpr float kPedestrianDrawDistanceFactor = kDrawDistanceFactor;
#endif

RenderKey createKey(bool transparent, float normalizedDepth,
                    Renderer::Textures& textures) {
    return ((transparent ? 0x1 : 0x0) << 31) |
           uint32_t(0x7FFFFF *
                    (transparent ? 1.f - normalizedDepth : normalizedDepth))
               << 8 |
           uint8_t(0xFF & (textures.size() > 0 ? textures[0] : 0)) << 0;
}

void ObjectRenderer::renderGeometry(Geometry* geom,
                                    const glm::mat4& modelMatrix,
                                    GameObject* object, RenderList& outList) {
    for (SubGeometry& subgeom : geom->subgeom) {
        bool isTransparent = false;

        Renderer::DrawParameters dp;

        dp.colour = {255, 255, 255, 255};
        dp.count = subgeom.numIndices;
        dp.start = subgeom.start;
        dp.textures = {0};
        dp.visibility = 1.f;

        if (object && object->type() == GameObject::Instance) {
            auto modelinfo = object->getModelInfo<SimpleModelInfo>();
            dp.depthWrite =
                !(modelinfo->flags & SimpleModelInfo::NO_ZBUFFER_WRITE);
        }

        if (geom->materials.size() > subgeom.material) {
            Geometry::Material& mat = geom->materials[subgeom.material];

            if (mat.textures.size() > 0) {
                auto tex = mat.textures[0].texture;
                if (tex) {
                    if (tex->isTransparent()) {
                        isTransparent = true;
                    }
                    dp.textures = {tex->getName()};
                }
            }

            if ((geom->flags & RW::BSGeometry::ModuleMaterialColor) ==
                RW::BSGeometry::ModuleMaterialColor) {
                dp.colour = mat.colour;

                if (object && object->type() == GameObject::Vehicle) {
                    auto vehicle = static_cast<VehicleObject*>(object);
                    if (dp.colour.r == 60 && dp.colour.g == 255 &&
                        dp.colour.b == 0) {
                        dp.colour = glm::u8vec4(vehicle->colourPrimary, 255);
                    } else if (dp.colour.r == 255 && dp.colour.g == 0 &&
                               dp.colour.b == 175) {
                        dp.colour = glm::u8vec4(vehicle->colourSecondary, 255);
                    }
                }
            }

            dp.visibility = 1.f;

            if (dp.colour.a < 255) {
                isTransparent = true;
            }

            dp.diffuse = mat.diffuseIntensity;
            dp.ambient = mat.ambientIntensity;
        }

        dp.blend = isTransparent;

        glm::vec3 position(modelMatrix[3]);
        float distance = glm::length(m_camera.position - position);
        float depth = (distance - m_camera.frustum.near) /
                      (m_camera.frustum.far - m_camera.frustum.near);
        outList.emplace_back(
            createKey(isTransparent, depth * depth, dp.textures), modelMatrix,
            &geom->dbuff, dp);
    }
}

void ObjectRenderer::renderAtomic(Atomic* atomic,
                                  const glm::mat4& worldtransform,
                                  GameObject* object, RenderList& render) {
    RW_CHECK(atomic->getGeometry(), "Can't render an atomic without geometry");
    RW_CHECK(atomic->getFrame(), "Can't render an atomic without a frame");

    const auto& geometry = atomic->getGeometry();
    const auto& frame = atomic->getFrame();

    RW::BSGeometryBounds& bounds = geometry->geometryBounds;

    glm::vec3 boundpos = bounds.center + glm::vec3(worldtransform[3]);
    if (!m_camera.frustum.intersects(boundpos, bounds.radius)) {
        culled++;
        return;
    }

    renderGeometry(geometry.get(), worldtransform, object, render);
}

void ObjectRenderer::renderClump(Clump* model, const glm::mat4& worldtransform,
                                 GameObject* object, RenderList& render) {
    for (const auto& atomic : model->getAtomics()) {
        const auto flags = atomic->getFlags();
        if ((flags & Atomic::ATOMIC_RENDER) == 0) {
            continue;
        }

        renderAtomic(atomic.get(), worldtransform, object, render);
    }
}

void ObjectRenderer::renderInstance(InstanceObject* instance,
                                    RenderList& outList) {
    if (!instance->getModel()) {
        return;
    }

    // Only draw visible objects
    if (!instance->getVisible()) {
        return;
    }

    auto modelinfo = instance->getModelInfo<SimpleModelInfo>();

    // Handles times provided by TOBJ data
    const auto currentHour = m_world->getHour();
    if (modelinfo->timeOff < modelinfo->timeOn) {
        if (currentHour >= modelinfo->timeOff &&
            currentHour < modelinfo->timeOn)
            return;
    } else {
        if (currentHour >= modelinfo->timeOff ||
            currentHour < modelinfo->timeOn)
            return;
    }

    auto matrixModel = instance->getTimeAdjustedTransform(m_renderAlpha);

    float mindist = glm::length(instance->getPosition() - m_camera.position) -
                    instance->getModel()->getBoundingRadius();
    mindist *= 1.f / kDrawDistanceFactor;

    Atomic* atomic = nullptr;

    // These are used to gracefully fade out things that are just out of
    // view
    // distance.
    Atomic* fadingAtomic = nullptr;
    auto fadingMatrix = matrixModel;
    float opacity = 0.f;
    constexpr float fadeRange = 50.f;

    /// @todo replace this block with the correct logic
    if (modelinfo->getNumAtomics() == 1) {
        // Is closest point greater than the *object* draw distance
        float objectRange = modelinfo->getLodDistance(0);
        float overlap = (mindist - objectRange);
        if (mindist > objectRange) {
            // Check for LOD instances
            if (instance->LODinstance) {
                // Is the closest point greater than the *LOD* draw distance
                auto lodmodelinfo =
                    instance->LODinstance->getModelInfo<SimpleModelInfo>();
                float LODrange = lodmodelinfo->getLodDistance(0);
                if (mindist <= LODrange && instance->LODinstance->getModel()) {
                    // The model matrix needs to be for the LOD instead
                    matrixModel =
                        instance->LODinstance->getTimeAdjustedTransform(
                            m_renderAlpha);
                    atomic = lodmodelinfo->getAtomic(0);
                    // If the object is only just out of range, keep
                    // rendering it and screen-door the LOD.
                    if (overlap < fadeRange) {
                        fadingAtomic = modelinfo->getAtomic(0);
                        opacity = 1.f - (overlap / fadeRange);
                    }
                }
            }
            // We don't have a LOD object, so fade out gracefully.
            else if (overlap < fadeRange) {
                fadingAtomic = modelinfo->getAtomic(0);
                opacity = 1.f - (overlap / fadeRange);
            }
        }
        // Otherwise, if we aren't marked as a LOD model, we can render
        else if (!modelinfo->LOD) {
            atomic = modelinfo->getAtomic(0);
        }
    } else {
        auto root = instance->getModel()->getFrame();

        matrixModel *= root->getTransform();

        for (int i = 0; i < modelinfo->getNumAtomics() - 1; ++i) {
            auto ind = (modelinfo->getNumAtomics() - 1) - i;
            float lodDistance = modelinfo->getLodDistance(i);
            if (mindist > lodDistance) {
                fadingAtomic = modelinfo->getAtomic(ind);
                opacity = 1.f - ((mindist - lodDistance) / fadeRange);
            } else {
                fadingAtomic = modelinfo->getAtomic(ind);
            }
        }
    }

    if (atomic) {
        renderAtomic(atomic, matrixModel, instance, outList);
    }
    if (fadingAtomic && opacity >= 0.01f) {
        // @todo pass opacity
        renderAtomic(fadingAtomic, fadingMatrix, instance, outList);
    }
}

void ObjectRenderer::renderCharacter(CharacterObject* pedestrian,
                                     RenderList& outList) {
    glm::mat4 matrixModel;

    if (pedestrian->getCurrentVehicle()) {
        auto vehicle = pedestrian->getCurrentVehicle();
        auto seat = pedestrian->getCurrentSeat();
        matrixModel = vehicle->getTimeAdjustedTransform(m_renderAlpha);
        if (pedestrian->isEnteringOrExitingVehicle()) {
            matrixModel = glm::translate(matrixModel,
                                         vehicle->getSeatEntryPosition(seat));
        } else {
            if (seat < vehicle->info->seats.size()) {
                matrixModel = glm::translate(matrixModel,
                                             vehicle->info->seats[seat].offset);
            }
        }
    } else {
        matrixModel = pedestrian->getTimeAdjustedTransform(m_renderAlpha);
    }

    if (!pedestrian->getModel()) return;

    renderClump(pedestrian->getModel(), matrixModel, nullptr, outList);

    auto item = pedestrian->getActiveItem();
    const auto& weapon = pedestrian->engine->data->weaponData[item];

    if (weapon->modelID == -1) {
        return;  // No model for this item
    }

    auto handFrame = pedestrian->getModel()->findFrame("srhand");
    glm::mat4 localMatrix;
    if (handFrame) {
        while (handFrame->getParent()) {
            localMatrix =
                pedestrian->skeleton->getMatrix(handFrame->getIndex()) *
                localMatrix;
            handFrame = handFrame->getParent();
        }
    }

    // Assume items are all simple
    auto simple =
        m_world->data->findModelInfo<SimpleModelInfo>(weapon->modelID);
    auto itematomic = simple->getAtomic(0);
    renderAtomic(itematomic, matrixModel * localMatrix, nullptr, outList);
}

void ObjectRenderer::renderVehicle(VehicleObject* vehicle,
                                   RenderList& outList) {
    RW_CHECK(vehicle->getModel(), "Vehicle model is null");

    if (!vehicle->getModel()) {
        return;
    }

    glm::mat4 matrixModel = vehicle->getTimeAdjustedTransform(m_renderAlpha);

    renderClump(vehicle->getModel(), matrixModel, vehicle, outList);

    auto modelinfo = vehicle->getVehicle();

    // Draw wheels n' stuff
    auto woi =
        m_world->data->findModelInfo<SimpleModelInfo>(modelinfo->wheelmodel_);
    if (!woi || !woi->isLoaded()) {
        return;
    }

    auto wheelatomic = woi->getAtomic(0);
    for (size_t w = 0; w < vehicle->info->wheels.size(); ++w) {
        auto& wi = vehicle->physVehicle->getWheelInfo(w);
        // Construct our own matrix so we can use the local transform
        vehicle->physVehicle->updateWheelTransform(w, false);
        /// @todo migrate this into Vehicle physics tick so we can
        /// interpolate old -> new

        glm::mat4 wheelM(matrixModel);

        auto up = -wi.m_wheelDirectionCS;
        auto right = wi.m_wheelAxleCS;
        auto fwd = up.cross(right);
        btQuaternion steerQ(up, wi.m_steering);
        btQuaternion rollQ(right, -wi.m_rotation);

        btMatrix3x3 basis(right[0], fwd[0], up[0], right[1], fwd[1], up[1],
                          right[2], fwd[2], up[2]);

        btTransform t;
        t.setBasis(btMatrix3x3(steerQ) * btMatrix3x3(rollQ) * basis);
        t.setOrigin(wi.m_chassisConnectionPointCS +
                    wi.m_wheelDirectionCS *
                        wi.m_raycastInfo.m_suspensionLength);

        t.getOpenGLMatrix(glm::value_ptr(wheelM));
        wheelM = matrixModel * wheelM;

        wheelM = glm::scale(wheelM, glm::vec3(modelinfo->wheelscale_));
        if (wi.m_chassisConnectionPointCS.x() < 0.f) {
            wheelM = glm::scale(wheelM, glm::vec3(-1.f, 1.f, 1.f));
        }

        renderAtomic(wheelatomic, wheelM, nullptr, outList);
    }
}

void ObjectRenderer::renderPickup(PickupObject* pickup, RenderList& outList) {
    if (!pickup->isEnabled()) return;

    glm::mat4 modelMatrix = glm::translate(glm::mat4(), pickup->getPosition());
    modelMatrix = glm::rotate(modelMatrix, m_world->getGameTime(),
                              glm::vec3(0.f, 0.f, 1.f));

    auto odata = pickup->getModelInfo<SimpleModelInfo>();

    auto atomic = odata->getAtomic(0);

    renderAtomic(atomic, modelMatrix, nullptr, outList);
}

void ObjectRenderer::renderCutsceneObject(CutsceneObject* cutscene,
                                          RenderList& outList) {
    if (!m_world->state->currentCutscene) return;

    if (!cutscene->getModel()) {
        return;
    }

    glm::mat4 matrixModel;
    auto cutsceneOffset = m_world->state->currentCutscene->meta.sceneOffset +
                          glm::vec3(0.f, 0.f, 1.f);

    if (cutscene->getParentActor()) {
        matrixModel = glm::translate(matrixModel, cutsceneOffset);
        // matrixModel =
        // cutscene->getParentActor()->getTimeAdjustedTransform(_renderAlpha);
        // matrixModel = glm::translate(matrixModel, glm::vec3(0.f, 0.f,
        // 1.f));
        glm::mat4 localMatrix;
        auto boneframe = cutscene->getParentFrame();
        while (boneframe) {
            localMatrix = cutscene->getParentActor()->skeleton->getMatrix(
                              boneframe->getIndex()) *
                          localMatrix;
            boneframe = boneframe->getParent();
        }
        matrixModel = matrixModel * localMatrix;
    } else {
        matrixModel = glm::translate(matrixModel, cutsceneOffset);
    }

    auto model = cutscene->getModel();
    if (cutscene->getParentActor()) {
        glm::mat4 align;
        /// @todo figure out where this 90 degree offset is coming from.
        align = glm::rotate(align, glm::half_pi<float>(), {0.f, 1.f, 0.f});
        renderClump(model, matrixModel * align, nullptr, outList);
    } else {
        renderClump(model, matrixModel, nullptr, outList);
    }
}

void ObjectRenderer::renderProjectile(ProjectileObject* projectile,
                                      RenderList& outList) {
    glm::mat4 modelMatrix = projectile->getTimeAdjustedTransform(m_renderAlpha);

    auto odata = m_world->data->findModelInfo<SimpleModelInfo>(
        projectile->getProjectileInfo().weapon->modelID);

    auto atomic = odata->getAtomic(0);
    renderAtomic(atomic, modelMatrix, nullptr, outList);
}

void ObjectRenderer::buildRenderList(GameObject* object, RenderList& outList) {
    if (object->skeleton) {
        object->skeleton->interpolate(m_renderAlpha);
    }

    // Right now specialized on each object type
    switch (object->type()) {
        case GameObject::Instance:
            renderInstance(static_cast<InstanceObject*>(object), outList);
            break;
        case GameObject::Character:
            renderCharacter(static_cast<CharacterObject*>(object), outList);
            break;
            ;
        case GameObject::Vehicle:
            renderVehicle(static_cast<VehicleObject*>(object), outList);
            break;
            ;
        case GameObject::Pickup:
            renderPickup(static_cast<PickupObject*>(object), outList);
            break;
        case GameObject::Projectile:
            renderProjectile(static_cast<ProjectileObject*>(object), outList);
            break;
        case GameObject::Cutscene:
            renderCutsceneObject(static_cast<CutsceneObject*>(object), outList);
            break;
        default:
            break;
    }
}
