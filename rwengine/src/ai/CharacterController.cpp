#include <btBulletDynamicsCommon.h>
#include <ai/CharacterController.hpp>
#include <data/Clump.hpp>
#include <engine/Animator.hpp>
#include <engine/GameData.hpp>
#include <engine/GameWorld.hpp>
#include <items/Weapon.hpp>
#include <objects/VehicleObject.hpp>

constexpr float kCloseDoorIdleTime = 2.f;

CharacterController::CharacterController(CharacterObject *character)
    : character(character)
    , _currentActivity(nullptr)
    , _nextActivity(nullptr)
    , m_closeDoorTimer(0.f)
    , currentGoal(None)
    , leader(nullptr)
    , targetNode(nullptr) {
    character->controller = this;
}

bool CharacterController::updateActivity() {
    if (_currentActivity && character->isAlive()) {
        return _currentActivity->update(character, this);
    }

    return false;
}

void CharacterController::setActivity(CharacterController::Activity *activity) {
    if (_currentActivity) delete _currentActivity;
    _currentActivity = activity;
}

void CharacterController::skipActivity() {
    // Some activities can't be cancelled, such as the final phase of entering a
    // vehicle
    // or jumping.
    if (getCurrentActivity() != nullptr &&
        getCurrentActivity()->canSkip(character, this)) {
        character->activityFinished();
        setActivity(nullptr);
    }
}

void CharacterController::setNextActivity(
    CharacterController::Activity *activity) {
    if (_currentActivity == nullptr) {
        setActivity(activity);
        _nextActivity = nullptr;
    } else {
        if (_nextActivity) delete _nextActivity;
        _nextActivity = activity;
    }
}

bool CharacterController::isCurrentActivity(const std::string &activity) const {
    if (getCurrentActivity() == nullptr) return false;
    return getCurrentActivity()->name() == activity;
}

void CharacterController::update(float dt) {
    if (character->isInVehicle()) {
        // Nevermind, the player is in a vehicle.

        auto &d = character->getMovement();

        if (character->isDriver()) {
            character->getCurrentVehicle()->setSteeringAngle(d.y);

            if (std::abs(d.x) > 0.01f) {
                character->getCurrentVehicle()->setHandbraking(false);
            }
            character->getCurrentVehicle()->setThrottle(d.x);
        }

        if (_currentActivity == nullptr) {
            // If character is idle in vehicle, try to close the door.
            auto v = character->getCurrentVehicle();
            auto entryDoor = v->getSeatEntryDoor(character->getCurrentSeat());

            if (entryDoor && entryDoor->constraint) {
                if (glm::length(d) <= 0.1f) {
                    if (m_closeDoorTimer >= kCloseDoorIdleTime) {
                        character->getCurrentVehicle()->setPartTarget(
                            entryDoor, true, entryDoor->closedAngle);
                    }
                    m_closeDoorTimer += dt;
                } else {
                    m_closeDoorTimer = 0.f;
                }
            }
        }
    } else {
        m_closeDoorTimer = 0.f;
    }

    if (updateActivity()) {
        character->activityFinished();
        if (_currentActivity) {
            delete _currentActivity;
            _currentActivity = nullptr;
        }
        if (_nextActivity) {
            setActivity(_nextActivity);
            _nextActivity = nullptr;
        }
    }
}

CharacterObject *CharacterController::getCharacter() const {
    return character;
}

void CharacterController::setMoveDirection(const glm::vec3 &movement) {
    character->setMovement(movement);
}

void CharacterController::setLookDirection(const glm::vec2 &look) {
    character->setLook(look);
}

void CharacterController::setRunning(bool run) {
    character->setRunning(run);
}
