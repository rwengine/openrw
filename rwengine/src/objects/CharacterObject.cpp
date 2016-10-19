#include <ai/CharacterController.hpp>
#include <data/Skeleton.hpp>
#include <engine/Animator.hpp>
#include <engine/GameData.hpp>
#include <engine/GameWorld.hpp>
#include <objects/CharacterObject.hpp>
#include <objects/VehicleObject.hpp>
#include <rw/defines.hpp>

// TODO: make this not hardcoded
static glm::vec3 enter_offset(0.81756252f, 0.34800607f, -0.486281008f);

const float CharacterObject::DefaultJumpSpeed = 2.f;

CharacterObject::CharacterObject(GameWorld* engine, const glm::vec3& pos,
                                 const glm::quat& rot, BaseModelInfo* modelinfo)
    : GameObject(engine, pos, rot, modelinfo)
    , currentState({})
    , currentVehicle(nullptr)
    , currentSeat(0)
    , running(false)
    , jumped(false)
    , jumpSpeed(DefaultJumpSpeed)
    , motionBlockedByActivity(false)
    , physCharacter(nullptr)
    , physObject(nullptr)
    , physShape(nullptr)
    , controller(nullptr) {
    // TODO move AnimationGroup creation somewhere else.
    animations.idle = engine->data->animations["idle_stance"];
    animations.walk = engine->data->animations["walk_player"];
    animations.walk_start = engine->data->animations["walk_start"];
    animations.run = engine->data->animations["run_player"];
    animations.sprint = engine->data->animations["sprint_civi"];

    animations.walk_right = engine->data->animations["walk_player_right"];
    animations.walk_right_start = engine->data->animations["walk_start_right"];
    animations.walk_left = engine->data->animations["walk_player_left"];
    animations.walk_left_start = engine->data->animations["walk_start_left"];

    animations.walk_back = engine->data->animations["walk_player_back"];
    animations.walk_back_start = engine->data->animations["walk_start_back"];

    animations.jump_start = engine->data->animations["jump_launch"];
    animations.jump_glide = engine->data->animations["jump_glide"];
    animations.jump_land = engine->data->animations["jump_land"];

    animations.car_sit = engine->data->animations["car_sit"];
    animations.car_sit_low = engine->data->animations["car_lsit"];

    animations.car_open_lhs = engine->data->animations["car_open_lhs"];
    animations.car_getin_lhs = engine->data->animations["car_getin_lhs"];
    animations.car_getout_lhs = engine->data->animations["car_getout_lhs"];
    animations.car_pullout_lhs = engine->data->animations["car_pullout_lhs"];
    animations.car_jacked_lhs = engine->data->animations["car_jackedlhs"];

    animations.car_open_rhs = engine->data->animations["car_open_rhs"];
    animations.car_getin_rhs = engine->data->animations["car_getin_rhs"];
    animations.car_getout_rhs = engine->data->animations["car_getout_rhs"];
    animations.car_pullout_rhs = engine->data->animations["car_pullout_rhs"];
    animations.car_jacked_rhs = engine->data->animations["car_jackedrhs"];

    animations.kd_front = engine->data->animations["kd_front"];
    animations.ko_shot_front = engine->data->animations["ko_shot_front"];

    auto info = getModelInfo<PedModelInfo>();
    if (info->getModel()) {
        setModel(info->getModel());
        skeleton = new Skeleton;
        animator = new Animator(getModel(), skeleton);

        createActor();
    }
}

CharacterObject::~CharacterObject() {
    destroyActor();
    if (currentVehicle) {
        currentVehicle->setOccupant(getCurrentSeat(), nullptr);
    }
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

        physObject = new btPairCachingGhostObject;
        physObject->setUserPointer(this);
        physObject->setWorldTransform(tf);
        physShape = new btCapsuleShapeZ(size.x, size.y);
        physObject->setCollisionShape(physShape);
        physObject->setCollisionFlags(btCollisionObject::CF_KINEMATIC_OBJECT);
        physCharacter =
            new btKinematicCharacterController(physObject, physShape, 0.30f, 2);
        physCharacter->setFallSpeed(20.f);
        physCharacter->setUseGhostSweepTest(true);
        physCharacter->setVelocityForTimeInterval(btVector3(1.f, 1.f, 1.f),
                                                  1.f);
        physCharacter->setGravity(engine->dynamicsWorld->getGravity().length());
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
        animator->playAnimation(0, animations.car_sit, 1.f, true);
        return glm::vec3();
    }

    Animation* movementAnimation = animations.idle;
    Animation* currentAnim = animator->getAnimation(AnimIndexMovement);
    bool isActionHappening =
        (animator->getAnimation(AnimIndexAction) != nullptr);
    float animationSpeed = 1.f;
    bool repeat = true;
    constexpr float movementEpsilon = 0.1f;

    float movementLength = glm::length(movement);
    if (!isAlive()) {
        movementAnimation = animations.ko_shot_front;
        repeat = false;
    } else if (jumped) {
        repeat = false;
        if (currentAnim == animations.jump_start &&
            animator->isCompleted(AnimIndexMovement)) {
            movementAnimation = animations.jump_start;
        }
        if (isOnGround()) {
            if (currentAnim != animations.jump_land ||
                !animator->isCompleted(AnimIndexMovement)) {
                movementAnimation = animations.jump_land;
            } else {
                // We are done jumping
                jumped = false;
            }
        } else {
            movementAnimation = animations.jump_glide;
        }
    } else if (movementLength > movementEpsilon) {
        if (running && !isActionHappening) {
            if (movementLength > 1.f) {
                movementAnimation = animations.sprint;
            } else {
                movementAnimation = animations.run;
            }
            animationSpeed = 1.f;
        } else {
            animationSpeed = 1.f / movementLength;
            // Determine if we need to play the walk start animation
            if (currentAnim != animations.walk) {
                if (currentAnim != animations.walk_start ||
                    !animator->isCompleted(AnimIndexMovement)) {
                    movementAnimation = animations.walk_start;
                } else {
                    movementAnimation = animations.walk;
                }
            } else {
                // Keep walkin
                movementAnimation = animations.walk;
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
    if (movementAnimation != animations.idle) {
        if (!getModel()->frames[0]->getChildren().empty()) {
            ModelFrame* root = getModel()->frames[0]->getChildren()[0];
            auto it = movementAnimation->bones.find(root->getName());
            if (it != movementAnimation->bones.end()) {
                AnimationBone* rootBone = it->second;
                float step = dt;
                const float duration =
                    animator->getAnimation(AnimIndexMovement)->duration;
                float animTime = fmod(
                    animator->getAnimationTime(AnimIndexMovement), duration);

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

                glm::vec3 a =
                    rootBone->getInterpolatedKeyframe(animTime).position;
                glm::vec3 b =
                    rootBone->getInterpolatedKeyframe(animTime + step).position;
                glm::vec3 d = (b - a);
                animTranslate.y += d.y;

                Skeleton::FrameData fd = skeleton->getData(root->getIndex());
                fd.a.translation.y = 0.f;
                skeleton->setData(root->getIndex(), fd);
            }
        }
    }

    return animTranslate;
}

void CharacterObject::tick(float dt) {
    if (controller) {
        controller->update(dt);
    }

    animator->tick(dt);
    updateCharacter(dt);

    // Ensure the character doesn't need to be reset
    if (getPosition().z < -100.f) {
        resetToAINode();
    }
}

#include <algorithm>
void CharacterObject::changeCharacterModel(const std::string& name) {
    auto modelName = std::string(name);
    std::transform(modelName.begin(), modelName.end(), modelName.begin(),
                   ::tolower);

    /// @todo don't model leak here

    engine->data->loadTXD(modelName + ".txd");
    auto newmodel = engine->data->loadClump(modelName + ".dff");

    if (skeleton) {
        delete animator;
        delete skeleton;
    }

    setModel(newmodel);

    skeleton = new Skeleton;
    animator = new Animator(getModel(), skeleton);
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
            rotation = glm::angleAxis(m_look.x, glm::vec3{0.f, 0.f, 1.f});
        }

        walkDir = rotation * walkDir;

        if (jumped) {
            if (!isOnGround()) {
                walkDir = rotation * glm::vec3(0.f, jumpSpeed * dt, 0.f);
            }
        }

        if (isAlive()) {
            physCharacter->setWalkDirection(
                btVector3(walkDir.x, walkDir.y, walkDir.z));
        } else {
            physCharacter->setWalkDirection(btVector3(0.f, 0.f, 0.f));
        }

        auto Pos =
            physCharacter->getGhostObject()->getWorldTransform().getOrigin();
        position = glm::vec3(Pos.x(), Pos.y(), Pos.z());

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

                physCharacter->setGravity(0.f);
                inWater = true;
            } else {
                physCharacter->setGravity(9.81f);
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
    return a == animations.car_getout_lhs || a == animations.car_getin_lhs ||
           a == animations.car_getout_rhs || a == animations.car_getin_rhs;
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
        physCharacter->jump();
        jumped = true;
        animator->playAnimation(AnimIndexMovement, animations.jump_start, 1.f,
                                false);
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

void CharacterObject::playActivityAnimation(Animation* animation, bool repeat,
                                            bool blocked) {
    RW_CHECK(animator != nullptr, "No Animator");
    animator->playAnimation(AnimIndexAction, animation, 1.f, repeat);
    motionBlockedByActivity = blocked;
}

void CharacterObject::activityFinished() {
    animator->playAnimation(AnimIndexAction, nullptr, 1.f, false);
    motionBlockedByActivity = false;
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
            if (currentState.weapons[j].weaponId != 0) {
                currentState.currentWeapon = j;
                return;
            }
        }

        // Nothing? set the highest
        for (int j = kMaxInventorySlots - 1; j >= 0; --j) {
            if (currentState.weapons[j].weaponId != 0 || j == 0) {
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
            }
            else if (currentState.primaryActive && !active) {
                // UseItem will cancel itself upon !primaryActive
            }
            currentState.primaryActive = active;
        } else {
            currentState.secondaryActive = active;
            /// @todo handle scopes and sights
        }
    }
}
