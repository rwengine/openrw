#include <ai/CharacterController.hpp>
#include <engine/Animator.hpp>
#include <engine/GameData.hpp>
#include <engine/GameWorld.hpp>
#include <objects/CharacterObject.hpp>
#include <objects/VehicleObject.hpp>
#include <rw/defines.hpp>

#include <algorithm>

// Required for BT_BULLET_VERSION
#include "LinearMath/btScalar.h"
#ifndef BT_BULLET_VERSION
#warning Unable to find BT_BULLET_VERSION
#endif

const float CharacterObject::DefaultJumpSpeed = 2.f;

CharacterObject::CharacterObject(GameWorld* engine, const glm::vec3& pos,
                                 const glm::quat& rot, BaseModelInfo* modelinfo,
                                 CharacterController* controller)
    : GameObject(engine, pos, rot, modelinfo)
    , currentState({})
    , currentVehicle(nullptr)
    , currentSeat(0)
    , m_look(0.f, glm::half_pi<float>())
    , running(false)
    , jumped(false)
    , jumpSpeed(DefaultJumpSpeed)
    , motionBlockedByActivity(false)
    , cycle_(AnimCycle::Idle)
    , physCharacter(nullptr)
    , physObject(nullptr)
    , physShape(nullptr)
    , controller(controller) {

    auto info = getModelInfo<PedModelInfo>();
    setClump(ClumpPtr(info->getModel()->clone()));
    if (info->getModel()) {
        setModel(info->getModel());
        animator = new Animator(getClump());

        createActor();
    }

    animations = engine->data->getAnimGroup(info->animgroup_);

    controller->character = this;
}

CharacterObject::~CharacterObject() {
    destroyActor();
    if (currentVehicle) {
        currentVehicle->setOccupant(getCurrentSeat(), nullptr);
    }
    delete controller;
}

void CharacterObject::createActor(const glm::vec2& size) {
    if (physCharacter) {
        destroyActor();
    }

    // Don't create anything without a valid model.
    if (getModel()) {
        btTransform tf;
        tf.setIdentity();
        tf.setOrigin(btVector3(position.x, position.y, position.z));

        physObject = new btPairCachingGhostObject();
        physObject->setUserPointer(this);
        physObject->setWorldTransform(tf);
        physShape = new btCapsuleShapeZ(size.x, size.y);
        physObject->setCollisionShape(physShape);
        physObject->setCollisionFlags(btCollisionObject::CF_KINEMATIC_OBJECT);
#if BT_BULLET_VERSION < 285
        physCharacter =
            new btKinematicCharacterController(physObject, physShape, 0.30f, 2);
#else
        physCharacter = new btKinematicCharacterController(
            physObject, physShape, 0.30f, btVector3(0.f, 0.f, 1.f));
#endif
        physCharacter->setFallSpeed(20.f);
        physCharacter->setUseGhostSweepTest(true);
        physCharacter->setVelocityForTimeInterval(btVector3(1.f, 1.f, 1.f),
                                                  1.f);
#if BT_BULLET_VERSION < 285
        physCharacter->setGravity(engine->dynamicsWorld->getGravity().length());
#else
        physCharacter->setGravity(engine->dynamicsWorld->getGravity());
#endif
        physCharacter->setJumpSpeed(5.f);

        engine->dynamicsWorld->addCollisionObject(
            physObject, btBroadphaseProxy::KinematicFilter,
            btBroadphaseProxy::StaticFilter | btBroadphaseProxy::SensorTrigger);
        engine->dynamicsWorld->addAction(physCharacter);
    }
}

void CharacterObject::destroyActor() {
    if (physCharacter) {
        engine->dynamicsWorld->removeCollisionObject(physObject);
        engine->dynamicsWorld->removeAction(physCharacter);

        delete physCharacter;
        delete physObject;
        delete physShape;
        physCharacter = nullptr;
    }
}

glm::vec3 CharacterObject::updateMovementAnimation(float dt) {
    glm::vec3 animTranslate;

    if (motionBlockedByActivity) {
        // Clear any residual motion animation
        animator->playAnimation(AnimIndexMovement, nullptr, 1.f, false);
        return glm::vec3();
    }

    // Things are simpler if we're in a vehicle
    if (getCurrentVehicle()) {
        animator->playAnimation(AnimIndexMovement, animations->animation(AnimCycle::CarSit),
                                1.f, true);
        return glm::vec3();
    }

    AnimationPtr movementAnimation = animations->animation(AnimCycle::Idle);
    AnimationPtr currentAnim = animator->getAnimation(AnimIndexMovement);
    bool isActionHappening =
        (animator->getAnimation(AnimIndexAction) != nullptr);
    float animationSpeed = 1.f;
    bool repeat = true;
    constexpr float movementEpsilon = 0.1f;

    float movementLength = glm::length(movement);
    if (!isAlive()) {
        movementAnimation =
            animations->animation(AnimCycle::KnockOutShotFront0);
        repeat = false;
    } else if (jumped) {
        repeat = false;
        if (currentAnim == animations->animation(AnimCycle::JumpLaunch) &&
            animator->isCompleted(AnimIndexMovement)) {
            movementAnimation = animations->animation(AnimCycle::JumpLaunch);
        }
        if (isOnGround()) {
            if (currentAnim != animations->animation(AnimCycle::JumpLand) ||
                !animator->isCompleted(AnimIndexMovement)) {
                movementAnimation = animations->animation(AnimCycle::JumpLand);
            } else {
                // We are done jumping
                jumped = false;
            }
        } else {
            movementAnimation = animations->animation(AnimCycle::JumpGlide);
        }
    } else if (movementLength > movementEpsilon) {
        if (running && !isActionHappening) {
            if (movementLength > 1.f) {
                movementAnimation = animations->animation(AnimCycle::Sprint);
            } else {
                movementAnimation = animations->animation(AnimCycle::Run);
            }
            animationSpeed = 1.f;
        } else {
            animationSpeed = 1.f / movementLength;
            // Determine if we need to play the walk start animation
            if (currentAnim != animations->animation(AnimCycle::Walk)) {
                if (currentAnim !=
                        animations->animation(AnimCycle::WalkStart) ||
                    !animator->isCompleted(AnimIndexMovement)) {
                    movementAnimation =
                        animations->animation(AnimCycle::WalkStart);
                } else {
                    movementAnimation = animations->animation(AnimCycle::Walk);
                }
            } else {
                // Keep walking
                movementAnimation = animations->animation(AnimCycle::Walk);
            }
        }
    }

    // Check if we need to change the animation or change speed
    if (animator->getAnimation(AnimIndexMovement) != movementAnimation) {
        animator->playAnimation(AnimIndexMovement, movementAnimation,
                                animationSpeed, repeat);
    } else {
        animator->setAnimationSpeed(AnimIndexMovement, animationSpeed);
    }

    // If we have to, interrogate the movement animation
    const auto& modelroot = getClump()->getFrame();
    if (movementAnimation != animations->animation(AnimCycle::Idle) &&
        !modelroot->getChildren().empty()) {
        const auto& root = modelroot->getChildren()[0];
        auto it = movementAnimation->bones.find(root->getName());
        if (it != movementAnimation->bones.end()) {
            AnimationBone* rootBone = it->second;
            float step = dt;
            const float duration =
                animator->getAnimation(AnimIndexMovement)->duration;
            float animTime =
                fmod(animator->getAnimationTime(AnimIndexMovement), duration);

            // Handle any remaining transformation before the end of the
            // keyframes
            if ((animTime + step) > duration) {
                glm::vec3 a =
                    rootBone->getInterpolatedKeyframe(animTime).position;
                glm::vec3 b =
                    rootBone->getInterpolatedKeyframe(duration).position;
                glm::vec3 d = (b - a);
                animTranslate.y += d.y;
                step -= (duration - animTime);
                animTime = 0.f;
            }

            glm::vec3 a = rootBone->getInterpolatedKeyframe(animTime).position;
            glm::vec3 b =
                rootBone->getInterpolatedKeyframe(animTime + step).position;
            glm::vec3 d = (b - a);
            animTranslate.y += d.y;

            // Kludge: Drop y component of root bone
            auto t = glm::vec3(root->getTransform()[3]);
            t.y = 0.f;
            root->setTranslation(t);
        }
    }

    return animTranslate;
}

void CharacterObject::tick(float dt) {
    if (controller) {
        controller->update(dt);
    }

    // Reset back to idle cycle when not in an activity
    if (controller->getCurrentActivity() == nullptr) {
        cycle_ = AnimCycle::Idle;
    }

    animator->tick(dt);
    updateCharacter(dt);

    // Ensure the character doesn't need to be reset
    if (getPosition().z < -100.f) {
        resetToAINode();
    }
}

void CharacterObject::tickPhysics(float dt) {
    if (physCharacter) {
        auto s = currenteMovementStep * dt;
        physCharacter->setWalkDirection(
            btVector3(s.x, s.y, s.z));
    }
}

void CharacterObject::setRotation(const glm::quat& orientation) {
    m_look.x = glm::roll(orientation);
    rotation = orientation;
    getClump()->getFrame()->setRotation(glm::mat3_cast(rotation));
}

void CharacterObject::changeCharacterModel(const std::string& name) {
    auto modelName = std::string(name);
    std::transform(modelName.begin(), modelName.end(), modelName.begin(),
                   ::tolower);

    /// @todo don't model leak here

    engine->data->loadTXD(modelName + ".txd");
    auto newmodel = engine->data->loadClump(modelName + ".dff");

    if (animator) {
        delete animator;
    }

    setModel(newmodel);

    animator = new Animator(getClump());
}

void CharacterObject::updateCharacter(float dt) {
    /*
     * You can fire weapons while moving
     *
     * Two Modes: Moving and Action
     * Moving covers walking & jumping
     * Action covers complex things like shooting, entering vehicles etc.
     *
     * Movement animation should be handled here
     *  If Current weapon is one handed, then it can be used while walking.
     *  This means blending the weapon animation with the walk animation.
     *  No weapons can be used while sprinting.
     *  Need an "aim vector" to apply torso correction.
     *
     *  If movement vector is less than some threshold, fully walk animation
     *  (time adjusted for velocity).
     */

    if (physCharacter) {
        glm::vec3 walkDir = updateMovementAnimation(dt);

        if (canTurn()) {
            float yaw = m_look.x;
            // When strafing we need to detach look direction from movement
            if (!isStrafing()) {
                yaw += std::atan2(movement.z, movement.x);
            }
            rotation = glm::quat(glm::vec3(0.f, 0.f, yaw));
            getClump()->getFrame()->setRotation(glm::mat3_cast(rotation));
        }

        walkDir = rotation * walkDir;

        if (jumped) {
            if (!isOnGround()) {
                walkDir = rotation * glm::vec3(0.f, jumpSpeed * dt, 0.f);
            }
        }

        if (isAlive()) {
            currenteMovementStep = walkDir / dt;
        } else {
            currenteMovementStep = glm::vec3();
        }

        auto Pos =
            physCharacter->getGhostObject()->getWorldTransform().getOrigin();
        position = glm::vec3(Pos.x(), Pos.y(), Pos.z());
        getClump()->getFrame()->setTranslation(position);

        // Handle above waist height water.
        auto wi = engine->data->getWaterIndexAt(getPosition());
        if (wi != NO_WATER_INDEX) {
            float wh = engine->data->waterHeights[wi];
            auto ws = getPosition();
            wh += engine->data->getWaveHeightAt(ws);

            // If Not in water before
            //  If last position was above water
            //   Now Underwater
            //  Else Not Underwater
            // Else
            //  Underwater

            if (!inWater && ws.z < wh && _lastHeight > wh) {
                ws.z = wh;

                btVector3 bpos(ws.x, ws.y, ws.z);
                physCharacter->warp(bpos);
                auto& wt = physObject->getWorldTransform();
                wt.setOrigin(bpos);
                physObject->setWorldTransform(wt);
#if BT_BULLET_VERSION < 285
                physCharacter->setGravity(0.f);
#else
                physCharacter->setGravity(btVector3(0.f, 0.f, 0.f));
#endif
                inWater = true;
            } else {
#if BT_BULLET_VERSION < 285
                physCharacter->setGravity(9.81f);
#else
                physCharacter->setGravity(btVector3(0.f, 0.f, -9.81f));
#endif
                inWater = false;
            }
        }
        _lastHeight = getPosition().z;
    } else {
        updateMovementAnimation(dt);
    }
}

void CharacterObject::setPosition(const glm::vec3& pos) {
    if (physCharacter) {
        btVector3 bpos(pos.x, pos.y, pos.z);
        if (std::abs(-100.f - pos.z) < 0.01f) {
            // Find the ground position
            auto gpos = engine->getGroundAtPosition(pos);
            bpos.setZ(gpos.z + 1.f);
        }
        physCharacter->warp(bpos);
    }
    position = pos;
    getClump()->getFrame()->setTranslation(pos);
}

bool CharacterObject::isPlayer() const {
    return engine->state->playerObject == getGameObjectID();
}

bool CharacterObject::isAlive() const {
    return currentState.health > 0.f;
}

bool CharacterObject::enterVehicle(VehicleObject* vehicle, size_t seat) {
    if (vehicle) {
        // Check that the seat is free
        if (vehicle->getOccupant(seat)) {
            return false;
        } else {
            // Make sure we leave any vehicle we're inside
            enterVehicle(nullptr, 0);
            vehicle->setOccupant(seat, this);
            setCurrentVehicle(vehicle, seat);
            // enterAction(VehicleSit);
            return true;
        }
    } else {
        if (currentVehicle) {
            currentVehicle->setOccupant(seat, nullptr);
            // Disabled due to crashing.
            // setPosition(currentVehicle->getPosition());
            setCurrentVehicle(nullptr, 0);
            return true;
        }
    }
    return false;
}

bool CharacterObject::isEnteringOrExitingVehicle() const {
    auto a = animator->getAnimation(AnimIndexAction);
    return a == animations->animation(AnimCycle::CarGetOutLHS) ||
           a == animations->animation(AnimCycle::CarGetInLHS) ||
           a == animations->animation(AnimCycle::CarGetOutRHS) ||
           a == animations->animation(AnimCycle::CarGetInRHS);
}

bool CharacterObject::isStopped() const {
    if (currentVehicle != nullptr) {
        return currentVehicle->isStopped();
    }

    return controller->getCurrentActivity() == nullptr;
}

VehicleObject* CharacterObject::getCurrentVehicle() const {
    return currentVehicle;
}

size_t CharacterObject::getCurrentSeat() const {
    return currentSeat;
}

void CharacterObject::setCurrentVehicle(VehicleObject* value, size_t seat) {
    currentVehicle = value;
    currentSeat = seat;
    if (currentVehicle == nullptr && physCharacter == nullptr) {
        createActor();
    } else if (currentVehicle) {
        destroyActor();
    }
}

bool CharacterObject::takeDamage(const GameObject::DamageInfo& dmg) {
    // Right now there's no state that determines immunity to any kind of damage
    float dmgPoints = dmg.hitpoints;
    if (currentState.armour > 0.f) {
        dmgPoints -= currentState.armour;
        currentState.armour =
            std::max(0.f, currentState.armour - dmg.hitpoints);
    }
    if (dmgPoints > 0.f) {
        currentState.health = std::max(0.f, currentState.health - dmgPoints);
    }
    return true;
}

void CharacterObject::jump() {
    if (physCharacter) {
#if BT_BULLET_VERSION < 285
        physCharacter->jump();
#else
        physCharacter->jump(btVector3(0.f, 0.f, 0.f));
#endif
        jumped = true;
        animator->playAnimation(AnimIndexMovement,
                                animations->animation(AnimCycle::JumpLaunch),
                                1.f, false);
    }
}

float CharacterObject::getJumpSpeed() const {
    return jumpSpeed;
}

bool CharacterObject::isOnGround() const {
    if (physCharacter) {
        return physCharacter->onGround();
    }
    return true;
}

bool CharacterObject::canTurn() const {
    return isOnGround() && !jumped && isAlive() &&
           controller->getCurrentActivity() == nullptr;
}

void CharacterObject::setJumpSpeed(float speed) {
    jumpSpeed = speed;
}

void CharacterObject::resetToAINode() {
    auto nodes = engine->aigraph.nodes;
    bool vehicleNode = !!getCurrentVehicle();
    AIGraphNode* nearest = nullptr;
    float d = std::numeric_limits<float>::max();
    for (auto it = nodes.begin(); it != nodes.end(); ++it) {
        if (vehicleNode) {
            if ((*it)->type == AIGraphNode::Pedestrian) continue;
        } else {
            if ((*it)->type == AIGraphNode::Vehicle) continue;
        }

        float dist = glm::length((*it)->position - getPosition());
        if (dist < d) {
            nearest = *it;
            d = dist;
        }
    }

    if (nearest) {
        if (vehicleNode) {
            getCurrentVehicle()->setPosition(nearest->position +
                                             glm::vec3(0.f, 0.f, 2.5f));
        } else {
            setPosition(nearest->position + glm::vec3(0.f, 0.f, 2.5f));
        }
    }
}

void CharacterObject::playActivityAnimation(AnimationPtr animation, bool repeat,
                                            bool blocked) {
    RW_CHECK(animator != nullptr, "No Animator");
    animator->playAnimation(AnimIndexAction, animation, 1.f, repeat);
    motionBlockedByActivity = blocked;
}

void CharacterObject::activityFinished() {
    animator->playAnimation(AnimIndexAction, nullptr, 1.f, false);
    motionBlockedByActivity = false;
}

void CharacterObject::playCycle(AnimCycle cycle) {
    auto animation = animations->animation(cycle);
    auto flags = animations->flags(cycle);

    cycle_ = cycle;
    animator->playAnimation(AnimIndexAction, animation, 1.f,
                            flags & AnimCycleInfo::Repeat);
}

void CharacterObject::playCycleAnimOverride(AnimCycle cycle, AnimationPtr anim) {
    auto flags = animations->flags(cycle);

    cycle_ = cycle;
    animator->playAnimation(AnimIndexAction, anim, 1.f,
                            flags & AnimCycleInfo::Repeat);
}

void CharacterObject::addToInventory(int slot, int ammo) {
    RW_CHECK(slot < kMaxInventorySlots, "Slot greater than kMaxInventorySlots");
    if (slot < kMaxInventorySlots) {
        currentState.weapons[slot].weaponId = slot;
        currentState.weapons[slot].bulletsTotal += ammo;
    }
}

void CharacterObject::setActiveItem(int slot) {
    currentState.currentWeapon = slot;
}

void CharacterObject::removeFromInventory(int slot) {
    currentState.weapons[slot].weaponId = 0;
    if (currentState.currentWeapon == slot) {
        currentState.currentWeapon = 0;
    }
}

void CharacterObject::cycleInventory(bool up) {
    if (up) {
        for (int j = currentState.currentWeapon + 1; j < kMaxInventorySlots;
             ++j) {
            if (currentState.weapons[j].weaponId != 0) {
                currentState.currentWeapon = j;
                return;
            }
        }

        // if there's no higher slot, set the first item.
        currentState.currentWeapon = 0;
    } else {
        for (int j = currentState.currentWeapon - 1; j >= 0; --j) {
            bool isFist = j == 0;
            if (currentState.weapons[j].weaponId != 0 || isFist) {
                currentState.currentWeapon = j;
                return;
            }
        }

        // Nothing? set the highest
        for (int j = kMaxInventorySlots - 1; j >= 0; --j) {
            bool isFist = j == 0;
            if (currentState.weapons[j].weaponId != 0 || isFist) {
                currentState.currentWeapon = j;
                return;
            }
        }
    }
}

void CharacterObject::useItem(bool active, bool primary) {
    /// @todo verify if this is the correct logic
    auto item = getActiveItem();
    if (currentState.weapons[item].weaponId == unsigned(item)) {
        if (primary) {
            if (!currentState.primaryActive && active) {
                // If we've just started, activate
                controller->setNextActivity(new Activities::UseItem(item));
            } else if (currentState.primaryActive && !active) {
                // UseItem will cancel itself upon !primaryActive
            }
            currentState.primaryActive = active;
        } else {
            currentState.secondaryActive = active;
            /// @todo handle scopes and sights
        }
    }
}
