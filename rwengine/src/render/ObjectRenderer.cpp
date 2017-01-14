#include <data/Clump.hpp>
#include <data/CutsceneData.hpp>
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

constexpr float kDrawDistanceFactor = 1.5f;
constexpr float kWorldDrawDistanceFactor = kDrawDistanceFactor;
constexpr float kVehicleDrawDistanceFactor = kDrawDistanceFactor;
#if 0  // There's no distance based culling for these types of objects yet
constexpr float kPedestrianDrawDistanceFactor = kDrawDistanceFactor;
#endif
constexpr float kMagicLODDistance = 330.f;
constexpr float kVehicleLODDistance = 70.f;
constexpr float kVehicleDrawDistance = 280.f;

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

    auto transform = worldtransform * frame->getWorldTransform();

    glm::vec3 boundpos = bounds.center + glm::vec3(transform[3]);
    if (!m_camera.frustum.intersects(boundpos, bounds.radius)) {
        culled++;
        return;
    }

    renderGeometry(geometry.get(), transform, object, render);
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
    const auto& atomic = instance->getAtomic();
    if (!atomic) {
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

    float mindist = glm::length(instance->getPosition() - m_camera.position) /
                    kDrawDistanceFactor;

    if (mindist > modelinfo->getLargestLodDistance()) {
        culled++;
        return;
    }

    if (modelinfo->isBigBuilding() &&
        mindist < modelinfo->getNearLodDistance() &&
        mindist < kMagicLODDistance) {
        auto related = modelinfo->related();
        if (!related || related->isLoaded()) {
            culled++;
            return;
        }
    }

    Atomic* distanceatomic =
        modelinfo->getDistanceAtomic(mindist / kDrawDistanceFactor);
    if (!distanceatomic) {
        return;
    }

    if (atomic->getGeometry() != distanceatomic->getGeometry()) {
        atomic->setGeometry(distanceatomic->getGeometry());
    }

    // Render the atomic the instance thinks it should be
    renderAtomic(atomic.get(), glm::mat4(), instance, outList);
}

void ObjectRenderer::renderCharacter(CharacterObject* pedestrian,
                                     RenderList& outList) {
    const auto& clump = pedestrian->getClump();

    if (pedestrian->getCurrentVehicle()) {
        auto vehicle = pedestrian->getCurrentVehicle();
        const auto& vehicleclump = vehicle->getClump();
        auto seat = pedestrian->getCurrentSeat();
        auto matrixModel = vehicleclump->getFrame()->getWorldTransform();
        if (pedestrian->isEnteringOrExitingVehicle()) {
            matrixModel = glm::translate(matrixModel,
                                         vehicle->getSeatEntryPosition(seat));
            clump->getFrame()->setTransform(matrixModel);
        } else {
            if (seat < vehicle->info->seats.size()) {
                matrixModel = glm::translate(matrixModel,
                                             vehicle->info->seats[seat].offset);
                clump->getFrame()->setTransform(matrixModel);
            }
        }
    }

    renderClump(pedestrian->getClump().get(), glm::mat4(), nullptr, outList);

    auto item = pedestrian->getActiveItem();
    const auto& weapon = pedestrian->engine->data->weaponData[item];

    if (weapon->modelID == -1) {
        return;  // No model for this item
    }

    auto handFrame = pedestrian->getClump()->findFrame("srhand");
    if (handFrame) {
        auto simple =
            m_world->data->findModelInfo<SimpleModelInfo>(weapon->modelID);
        auto itematomic = simple->getAtomic(0);
        renderAtomic(itematomic, handFrame->getWorldTransform(), nullptr,
                     outList);
    }
}

void ObjectRenderer::renderVehicle(VehicleObject* vehicle,
                                   RenderList& outList) {
    const auto& clump = vehicle->getClump();
    RW_CHECK(clump, "Vehicle clump is null");
    if (!clump) {
        return;
    }

    float mindist = glm::length(vehicle->getPosition() - m_camera.position) / kVehicleDrawDistanceFactor;
    if (mindist < kVehicleLODDistance) {
        // Swich visibility to the high LOD
        vehicle->getHighLOD()->setFlag(Atomic::ATOMIC_RENDER, true);
        vehicle->getLowLOD()->setFlag(Atomic::ATOMIC_RENDER, false);
    } else if (mindist < kVehicleDrawDistance) {
        // Switch to low
        vehicle->getHighLOD()->setFlag(Atomic::ATOMIC_RENDER, false);
        vehicle->getLowLOD()->setFlag(Atomic::ATOMIC_RENDER, true);
    } else {
        culled++;
        return;
    }

    renderClump(clump.get(), glm::mat4(), vehicle, outList);

    auto modelinfo = vehicle->getVehicle();
    auto woi =
        m_world->data->findModelInfo<SimpleModelInfo>(modelinfo->wheelmodel_);
    if (!woi || !woi->isLoaded() || !woi->getDistanceAtomic(mindist)) {
        return;
    }

    auto wheelatomic = woi->getDistanceAtomic(mindist);
    for (size_t w = 0; w < vehicle->info->wheels.size(); ++w) {
        auto& wi = vehicle->physVehicle->getWheelInfo(w);
        // Construct our own matrix so we can use the local transform
        vehicle->physVehicle->updateWheelTransform(w, false);

        auto up = -wi.m_wheelDirectionCS;
        auto right = wi.m_wheelAxleCS;
        auto fwd = up.cross(right);
        btQuaternion steerQ(up, wi.m_steering);
        btQuaternion rollQ(right, -wi.m_rotation);
        btMatrix3x3 basis(right[0], fwd[0], up[0], right[1], fwd[1], up[1],
                          right[2], fwd[2], up[2]);
        btTransform t(
            btMatrix3x3(steerQ) * btMatrix3x3(rollQ) * basis,
            wi.m_chassisConnectionPointCS +
                wi.m_wheelDirectionCS * wi.m_raycastInfo.m_suspensionLength);
        glm::mat4 wheelM;
        t.getOpenGLMatrix(glm::value_ptr(wheelM));
        wheelM = clump->getFrame()->getWorldTransform() * wheelM;
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
    const auto& clump = cutscene->getClump();

    auto cutsceneOffset = m_world->state->currentCutscene->meta.sceneOffset +
                          glm::vec3(0.f, 0.f, 1.f);
    glm::mat4 cutscenespace;

    cutscenespace = glm::translate(cutscenespace, cutsceneOffset);
    if (cutscene->getParentActor()) {
        auto parent = cutscene->getParentFrame();
        cutscenespace *= parent->getWorldTransform();
        cutscenespace =
            glm::rotate(cutscenespace, glm::half_pi<float>(), {0.f, 1.f, 0.f});
    }

    renderClump(clump.get(), cutscenespace, nullptr, outList);
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
