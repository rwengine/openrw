#ifndef _RWENGINE_CHARACTEROBJECT_HPP_
#define _RWENGINE_CHARACTEROBJECT_HPP_

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>

#include <rw/forward.hpp>

#include <data/AnimGroup.hpp>
#include <objects/GameObject.hpp>

namespace ai {
class CharacterController;
}

class BaseModelInfo;
class btCapsuleShapeZ;
class btKinematicCharacterController;
class btPairCachingGhostObject;

constexpr int kMaxInventorySlots = 13;

// Animation slots used for character animation blending
constexpr unsigned int AnimIndexMovement = 0;
constexpr unsigned int AnimIndexAction = 1;

struct CharacterWeaponSlot {
    // Assuming these match the entries in weapon.dat
    uint32_t weaponId;
    uint32_t bulletsClip;
    uint32_t bulletsTotal;
};

struct CharacterState {
    float health = 100.f;
    float armour = 0.f;
    bool isDying = false;
    bool isDead = false;
    std::array<CharacterWeaponSlot, kMaxInventorySlots> weapons;
    uint16_t currentWeapon = 0;
    uint32_t lastFireTimeMS = 0;
    bool primaryActive = false;
    bool secondaryActive = false;
};

class VehicleObject;
class GameWorld;

/**
 * @brief The CharacterObject struct
 * Implements Character object behaviours.
 */
class CharacterObject final : public GameObject, public ClumpObject {
private:
    CharacterState currentState{};

    VehicleObject* currentVehicle = nullptr;
    size_t currentSeat{0};

    void createActor(const glm::vec2& size = glm::vec2(0.45f, 1.2f));
    void destroyActor();

    glm::vec3 movement{};
    glm::vec2 m_look{0.f, glm::half_pi<float>()};

    bool running = false;
    bool jumped = false;
    AnimationPtr jumpAnimation = nullptr;
    float jumpSpeed = DefaultJumpSpeed;

    bool motionBlockedByActivity = false;

    glm::vec3 updateMovementAnimation(float dt);
    glm::vec3 currenteMovementStep{};

    AnimCycle cycle_ = AnimCycle::Idle;

public:
    static const float DefaultJumpSpeed;

    std::unique_ptr<btKinematicCharacterController> physCharacter;
    std::unique_ptr<btPairCachingGhostObject> physObject;
    std::unique_ptr<btCapsuleShapeZ> physShape;

    ai::CharacterController* controller;

    AnimGroup* animations;

    /**
     * @param pos
     * @param rot
     * @param model
     * @param ped PEDS_t struct to use.
     */
    CharacterObject(GameWorld* engine, const glm::vec3& pos,
                    const glm::quat& rot, BaseModelInfo* modelinfo,
                    ai::CharacterController* controller);

    ~CharacterObject() override;

    Type type() const override {
        return Character;
    }

    void tick(float dt) override;

    void tickPhysics(float dt);

    const CharacterState& getCurrentState() const {
        return currentState;
    }
    CharacterState& getCurrentState() {
        return currentState;
    }

    void setRotation(const glm::quat& orientation) override;

    /**
     * @brief Loads the model and texture for a character skin.
     */
    void changeCharacterModel(const std::string& name);

    /**
     * @brief updateCharacter updates internall bullet Character.
     */
    void updateCharacter(float dt);

    void setPosition(const glm::vec3& pos) override;

    glm::vec3 getCenterOffset() override;

    bool isPlayer() const;

    bool isDying() const;
    bool isDead() const;
    bool isAlive() const;

    void Die();
    void SetDead();

    bool isKnockedDown() const;

    bool takeDamage(const DamageInfo& damage) override;

    bool enterVehicle(VehicleObject* vehicle, size_t seat);

    bool isEnteringOrExitingVehicle() const;

    /**
     * @brief isStopped
     * @return True if the character isn't moving
     */
    bool isStopped() const;

    VehicleObject* getCurrentVehicle() const;
    size_t getCurrentSeat() const;
    void setCurrentVehicle(VehicleObject* value, size_t seat);

    void jump();
    void setJumpSpeed(float speed);
    float getJumpSpeed() const;
    bool isOnGround() const;
    bool canTurn() const;

    void setRunning(bool run) {
        running = run;
    }
    bool isRunning() const {
        return running;
    }

    bool isStrafing() const {
        /// @todo implement strafing
        return false;
    }

    /**
     * Resets the Actor to the nearest AI Graph node
     * (taking into account the current vehicle)
     */
    void resetToAINode();

    void setMovement(const glm::vec3& _m) {
        movement = _m;
    }
    const glm::vec3& getMovement() const {
        return movement;
    }
    void setLook(const glm::vec2& look) {
        m_look = look;
    }
    const glm::vec2& getLook() const {
        return m_look;
    }

    glm::vec3 getLookDirection() const {
        float theta = m_look.y - glm::half_pi<float>();
        return glm::vec3(std::sin(-m_look.x) * std::cos(theta),
                         std::cos(-m_look.x) * std::cos(theta),
                         std::sin(theta));
    }

    /**
     * @brief playActivityAnimation Plays an animation for an activity.
     * @param animation The animation to play
     * @param repeat
     * @param blocking Wether movement is still alowed
     *
     * This allows controller activities to play their own animations and
     * controll blending with movement.
     */
    void playActivityAnimation(const AnimationPtr& animation, bool repeat,
                               bool blocking);
    /**
     * @brief activityFinished removes activity animation
     */
    void activityFinished();

    /**
     * Play the animation from the ped's animation group
     *
     * Flags on the cycle in the anim group will control repeating etc.
     */
    void playCycle(AnimCycle cycle);

    /**
     * Play the given animation, with the flags of the given cycle
     *
     * This sets the same state as playCycle, but provides an alternate
     * animation to play.
     */
    void playCycleAnimOverride(AnimCycle cycle, const AnimationPtr& anim);

    AnimCycle getCurrentCycle() const {
        return cycle_;
    }

    /**
     * @brief addToInventory Adds ammo to the specified item slot
     * @param slot The slot to add ammo for
     * @param ammo The quanity of ammunition to add
     *
     * Will give the weapon (set the ID) if it is not possessed
     */
    void addToInventory(int slot, int ammo);

    void setActiveItem(int slot);
    int getActiveItem() const {
        return currentState.currentWeapon;
    }

    /**
     * @brief removeFromInventory Removes item at slot from inventory
     */
    void removeFromInventory(int slot);

    /**
     * Uses the character's active item.
     * @param primary use the primary action.
     */
    void useItem(bool active, bool primary = true);

    void cycleInventory(bool up);

    void clearInventory();
};

#endif
