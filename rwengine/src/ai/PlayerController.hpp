#ifndef _RWENGINE_PLAYERCONTROLLER_HPP_
#define _RWENGINE_PLAYERCONTROLLER_HPP_

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <ai/CharacterController.hpp>

class PlayerController : public CharacterController {
    glm::quat cameraRotation{1.0f,0.0f,0.0f,0.0f};

    glm::vec3 direction{};

    glm::quat lastRotation;

    bool _enabled;

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

    void update(float dt) override;

    glm::vec3 getTargetPosition() override;

    void jump();

    /**
     * returns 0 (only one player supported)
     */
    int getScriptObjectID() const {
        return 0;
    }
};

#endif
