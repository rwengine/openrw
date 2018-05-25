#ifndef _RWENGINE_PLAYERCONTROLLER_HPP_
#define _RWENGINE_PLAYERCONTROLLER_HPP_

#include <ai/CharacterController.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class PlayerController : public CharacterController {
    glm::quat cameraRotation{1.0f, 0.0f, 0.0f, 0.0f};

    glm::vec3 direction{};

    glm::quat lastRotation;

    bool missionRestartRequired;

    bool adrenalineEffect;
    float adrenalineEffectTime;

    bool _enabled;

    enum RestartState {
        Alive,
        FadingIn,
        Restarting,
        FadingOut,
    } restartState;

    // handles player respawn logic
    void restartLogic();

    void restart();

public:
    PlayerController();

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
