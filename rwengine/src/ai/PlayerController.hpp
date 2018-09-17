#ifndef _RWENGINE_PLAYERCONTROLLER_HPP_
#define _RWENGINE_PLAYERCONTROLLER_HPP_

#include <ai/CharacterController.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class PlayerController final : public CharacterController {
private:
    glm::quat cameraRotation{1.0f, 0.0f, 0.0f, 0.0f};

    glm::vec3 direction{};

    bool missionRestartRequired = false;

    bool adrenalineEffect = false;
    float adrenalineEffectTime{0};

    bool _enabled = true;

    enum RestartState {
        Alive,
        FadingOut,
        Restarting,
        FadingIn
    } restartState = Alive;

    enum PayphoneState {
        Left,
        Talking,
        PickingUp,
        HangingUp,
    } payphoneState = Left;

    // handles player respawn logic
    void restartLogic();

    void restart();

public:
    PlayerController() = default;

    /**
     * @brief Enables and disables player input.
     * @todo actually implement input being disabled.
     */
    void setInputEnabled(bool enabled);
    bool isInputEnabled() const;

    void updateCameraDirection(const glm::quat& rot);

    void updateMovementDirection(const glm::vec3& pos,
                                 const glm::vec3& rawdirection);

    void exitVehicle();

    void enterNearestVehicle();

    void requestMissionRestart();

    bool isMissionRestartRequired() const;

    bool isWasted() const;
    // @todo not implemented yet
    bool isBusted() const;

    // Play payphone pick up anim
    void pickUpPayphone();
    // Play payphone hang up anim
    void hangUpPayphone();
    // Play talking on payphone anim
    void talkOnPayphone();
    // Reset any payphone anim
    void leavePayphone();

    // Is payphone pick up anim playing
    bool isPickingUpPayphone() const;
    // Is payphone hang up anim playing
    bool isHangingUpPayphone() const;
    // Is talking on payphone anim playing
    bool isTalkingOnPayphone() const;

    // @todo not implemented yet
    // makes player invincible, ignored by police, clears all particles,
    // projectiles, extinguish all fires, including cars on fire
    void prepareForCutscene();
    // @todo not implemented yet
    // makes player no longer invincible and not ignored by police
    void freeFromCutscene();

    void update(float dt) override;

    glm::vec3 getTargetPosition() override;

    bool isAdrenalineActive() {
        return adrenalineEffect;
    };

    void activateAdrenalineEffect();

    void jump();

    /**
     * returns 0 (only one player supported)
     */
    int getScriptObjectID() const {
        return 0;
    }
};

#endif
