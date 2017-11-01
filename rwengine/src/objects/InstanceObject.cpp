#include "objects/InstanceObject.hpp"

#include <cstdint>
#include <string>

#include <btBulletDynamicsCommon.h>
#include <glm/gtc/quaternion.hpp>

#include <rw/types.hpp>

#include "dynamics/CollisionInstance.hpp"
#include "engine/Animator.hpp"
#include "engine/GameData.hpp"
#include "engine/GameWorld.hpp"

InstanceObject::InstanceObject(GameWorld* engine, const glm::vec3& pos,
                               const glm::quat& rot, const glm::vec3& scale,
                               BaseModelInfo* modelinfo,
                               std::shared_ptr<DynamicObjectData> dyn)
    : GameObject(engine, pos, rot, modelinfo)
    , health(100.f)
    , scale(scale)
    , body(nullptr)
    , dynamics(dyn)
    , _enablePhysics(false) {
    if (modelinfo) {
        changeModel(modelinfo);
        setPosition(pos);
        setRotation(rot);

        // Disable the main island LOD
        if (modelinfo->name.find("IslandLOD") != std::string::npos) {
            setVisible(false);
        }

        /// @todo store path information properly
        if (modelinfo->type() == ModelDataType::SimpleInfo) {
            auto simpledata = static_cast<SimpleModelInfo*>(modelinfo);
            for (auto& path : simpledata->paths) {
                engine->aigraph.createPathNodes(position, rot, path);
            }
        }
    }
}

InstanceObject::~InstanceObject() {
}

void InstanceObject::tick(float dt) {
    if (dynamics && body) {
        if (_enablePhysics) {
            if (body->getBulletBody()->isStaticObject()) {
                // Apparently bodies must be removed and re-added if their mass
                // changes.
                body->changeMass(dynamics->mass);
            }
        }

        const glm::vec3& ws = getPosition();
        auto wX = (int)((ws.x + WATER_WORLD_SIZE / 2.f) /
                        (WATER_WORLD_SIZE / WATER_HQ_DATA_SIZE));
        auto wY = (int)((ws.y + WATER_WORLD_SIZE / 2.f) /
                        (WATER_WORLD_SIZE / WATER_HQ_DATA_SIZE));
        float vH = ws.z;  // - _collisionHeight/2.f;
        float wH = 0.f;

        if (wX >= 0 && wX < WATER_HQ_DATA_SIZE && wY >= 0 &&
            wY < WATER_HQ_DATA_SIZE) {
            int i = (wX * WATER_HQ_DATA_SIZE) + wY;
            int hI = engine->data->realWater[i];
            if (hI < NO_WATER_INDEX) {
                wH = engine->data->waterHeights[hI];
                wH += engine->data->getWaveHeightAt(ws);
                inWater = vH <= wH;
            } else {
                inWater = false;
            }
        }
        _lastHeight = ws.z;

        if (inWater) {
            float oZ =
                -(body->getBoundingHeight() * (dynamics->bouancy / 100.f));
            body->getBulletBody()->activate(true);
            // Damper motion
            body->getBulletBody()->setDamping(0.95f, 0.9f);

            auto wi = engine->data->getWaterIndexAt(ws);
            if (wi != NO_WATER_INDEX) {
                float h = engine->data->waterHeights[wi] + oZ;

                // Calculate wave height
                h += engine->data->getWaveHeightAt(ws);

                if (ws.z <= h) {
                    /*if( dynamics->uprootForce > 0.f &&
                    (body->body->getCollisionFlags() &
                    btRigidBody::CF_STATIC_OBJECT) != 0 ) {
                        // Apparently bodies must be removed and re-added if
                    their mass changes.
                        engine->dynamicsWorld->removeRigidBody(body->body);
                        btVector3 inert;
                        body->getCollisionShape()->calculateLocalInertia(dynamics->mass,
                    inert);
                        body->setMassProps(dynamics->mass, inert);
                        engine->dynamicsWorld->addRigidBody(body);
                    }*/

                    float x = (h - ws.z);
                    float F =
                        WATER_BUOYANCY_K * x +
                        -WATER_BUOYANCY_C *
                            body->getBulletBody()->getLinearVelocity().z();
                    btVector3 forcePos = btVector3(0.f, 0.f, 2.f)
                                             .rotate(body->getBulletBody()
                                                         ->getOrientation()
                                                         .getAxis(),
                                                     body->getBulletBody()
                                                         ->getOrientation()
                                                         .getAngle());
                    body->getBulletBody()->applyForce(btVector3(0.f, 0.f, F),
                                                      forcePos);
                }
            }
        }
    }

    if (animator) animator->tick(dt);
}

void InstanceObject::changeModel(BaseModelInfo* incoming) {
    if (body) {
        body.reset();
    }

    if (incoming) {
        if (!incoming->isLoaded()) {
            engine->data->loadModel(incoming->id());
        }

        changeModelInfo(incoming);
        /// @todo this should only be temporary
        setModel(getModelInfo<SimpleModelInfo>()->getModel());
        auto collision = getModelInfo<SimpleModelInfo>()->getCollision();

        auto modelatomic = getModelInfo<SimpleModelInfo>()->getAtomic(0);
        if (modelatomic) {
            auto previousatomic = atomic_;
            atomic_ = modelatomic->clone();
            if (previousatomic) {
                atomic_->setFrame(previousatomic->getFrame());
            } else {
                atomic_->setFrame(std::make_shared<ModelFrame>());
            }
        }

        if (collision) {
            body = std::make_unique<CollisionInstance>();
            body->createPhysicsBody(this, collision, dynamics.get());
            body->getBulletBody()->setActivationState(ISLAND_SLEEPING);
        }
    }
}

void InstanceObject::setPosition(const glm::vec3& pos) {
    if (body) {
        auto& wtr = body->getBulletBody()->getWorldTransform();
        wtr.setOrigin(btVector3(pos.x, pos.y, pos.z));
    }
    if (atomic_) {
        atomic_->getFrame()->setTranslation(pos);
    }
    GameObject::setPosition(pos);
}

void InstanceObject::setRotation(const glm::quat& r) {
    if (body) {
        auto& wtr = body->getBulletBody()->getWorldTransform();
        wtr.setRotation(btQuaternion(r.x, r.y, r.z, r.w));
    }
    if (atomic_) {
        atomic_->getFrame()->setRotation(glm::mat3_cast(r));
    }
    GameObject::setRotation(r);
}

bool InstanceObject::takeDamage(const GameObject::DamageInfo& dmg) {
    bool smash = false;
    if (dynamics) {
        smash = dynamics->collDamageFlags == 80;

        if (dmg.impulse >= dynamics->uprootForce &&
            (body->getBulletBody()->getCollisionFlags() &
             btRigidBody::CF_STATIC_OBJECT) != 0) {
            _enablePhysics = true;
        }
    }
    if (smash) {
        health -= dmg.hitpoints;
        return true;
    }
    return false;
}

void InstanceObject::setSolid(bool solid) {
    // Early out in case we don't have a collision body
    if (body == nullptr || body->getBulletBody() == nullptr) {
        return;
    }

    int flags = body->getBulletBody()->getCollisionFlags();
    if (solid) {
        flags &= ~btCollisionObject::CF_NO_CONTACT_RESPONSE;
    } else {
        flags |= btCollisionObject::CF_NO_CONTACT_RESPONSE;
    }
    body->getBulletBody()->setCollisionFlags(flags);
}

void InstanceObject::updateTransform(const glm::vec3& pos,
                                     const glm::quat& rot) {
    position = pos;
    rotation = rot;
    getAtomic()->getFrame()->setRotation(glm::mat3_cast(rot));
    getAtomic()->getFrame()->setTranslation(pos);
}
