#include <ai/PlayerController.hpp>
#include <engine/Animator.hpp>
#include <engine/GameWorld.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <objects/CharacterObject.hpp>
#include <objects/VehicleObject.hpp>

PlayerController::PlayerController()
    : CharacterController()
    , lastRotation(glm::vec3(0.f, 0.f, 0.f))
    , _enabled(true) {
}

void PlayerController::setInputEnabled(bool enabled) {
    _enabled = enabled;
}

bool PlayerController::isInputEnabled() const {
    return _enabled;
}

void PlayerController::updateCameraDirection(const glm::quat& rot) {
    cameraRotation = rot;
}

void PlayerController::updateMovementDirection(const glm::vec3& dir,
                                               const glm::vec3& rawdirection) {
    if (_currentActivity == nullptr) {
        direction = dir;
        setMoveDirection(rawdirection);
    }
}

void PlayerController::exitVehicle() {
    if (character->getCurrentVehicle()) {
        setNextActivity(std::make_unique<Activities::ExitVehicle>());
    }
}

void PlayerController::enterNearestVehicle() {
    if (!character->getCurrentVehicle()) {
        auto world = character->engine;
        VehicleObject* nearest = nullptr;
        float d = 10.f;

        for (auto& p : world->vehiclePool.objects) {
            auto object = p.second;
            float vd =
                glm::length(character->getPosition() - object->getPosition());
            if (vd < d) {
                d = vd;
                nearest = static_cast<VehicleObject*>(object);
            }
        }

        if (nearest) {
            setNextActivity(std::make_unique<Activities::EnterVehicle>(nearest, 0));
        }
    }
}

void PlayerController::update(float dt) {
    CharacterController::update(dt);
}

glm::vec3 PlayerController::getTargetPosition() {
    return direction;
}

void PlayerController::jump() {
    if (!character->isInWater()) {
        setNextActivity(std::make_unique<Activities::Jump>());
    }
}
