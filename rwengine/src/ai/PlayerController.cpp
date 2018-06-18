#include "ai/PlayerController.hpp"

#include <memory>

#include <glm/gtc/quaternion.hpp>

#include "engine/Animator.hpp"
#include "engine/GameState.hpp"
#include "engine/GameWorld.hpp"
#include "objects/CharacterObject.hpp"
#include "objects/GameObject.hpp"
#include "objects/VehicleObject.hpp"

class Animator;

PlayerController::PlayerController()
    : CharacterController()
    , lastRotation(glm::vec3(0.f, 0.f, 0.f))
    , missionRestartRequired(false)
    , _enabled(true)
    , restartState(Alive)
    , payphoneState(Left) {
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
            setNextActivity(
                std::make_unique<Activities::EnterVehicle>(nearest, 0));
        }
    }
}

void PlayerController::requestMissionRestart() {
    missionRestartRequired = true;
}

bool PlayerController::isMissionRestartRequired() const {
    return missionRestartRequired;
}

bool PlayerController::isWasted() const {
    return character->isDead();
}

bool PlayerController::isBusted() const {
    return false;
}

void PlayerController::restart() {
    GameWorld* world = character->engine;
    GameState* state = character->engine->state;

    glm::vec3 restartPosition;
    float restartHeading = 0.f;

    if (state->overrideNextRestart) {
        restartPosition = glm::vec3(state->nextRestartLocation);
        restartHeading = state->nextRestartLocation.w;
    } else {
        GameState::RestartType type{};

        if (isBusted()) {
            type = GameState::RestartType::Police;
        }

        if (isWasted()) {
            type = GameState::RestartType::Hospital;
        }

        glm::vec4 closestRestart =
            state->getClosestRestart(type, character->getPosition());
        restartPosition = glm::vec3(closestRestart);
        restartHeading = closestRestart.w;
    }

    if (isWasted()) {
        if (!state->playerInfo.singlePayerHealthcare) {
            if (state->playerInfo.money >= 1000) {
                state->playerInfo.money -= 1000;
            } else {
                state->playerInfo.money = 0;
            }
        } else {
            state->playerInfo.singlePayerHealthcare = false;
        }

        state->gameStats.timesHospital++;
    }

    if (isBusted()) {
        if (!state->playerInfo.thaneOfLibertyCity) {
            // @todo implement wanted system
            uint8_t wantedLevel = 0;
            int32_t penalty = 0;

            switch (wantedLevel) {
                case 0:
                    penalty = 100;
                    break;
                case 1:
                    penalty = 100;
                    break;
                case 2:
                    penalty = 200;
                    break;
                case 3:
                    penalty = 400;
                    break;
                case 4:
                    penalty = 600;
                    break;
                case 5:
                    penalty = 900;
                    break;
                case 6:
                    penalty = 1500;
                    break;
            }

            if (state->playerInfo.money >= penalty) {
                state->playerInfo.money -= penalty;
            } else {
                state->playerInfo.money = 0;
            }
        } else {
            state->playerInfo.thaneOfLibertyCity = false;
        }

        state->gameStats.timesBusted++;
    }

    // Clean up mission restart
    missionRestartRequired = false;

    // Clean up overrides
    state->hospitalIslandOverride = 0;
    state->policeIslandOverride = 0;

    // If we are in vehicle get us out
    auto* vehicle = character->getCurrentVehicle();
    if (vehicle) {
        auto seat = character->getCurrentSeat();
        character->setCurrentVehicle(nullptr, seat);
        vehicle->setOccupant(seat, nullptr);
    }

    // Clear whole world
    // Original game uses 4000.f radius
    world->clearObjectsWithinArea(character->getPosition(), 10000.f, true);

    // Set position and heading for any restart
    character->setPosition(restartPosition);
    character->setHeading(restartHeading);

    if (isWasted() || isBusted()) {
        // Advance 12 hours
        world->offsetGameTime(60 * 12);

        character->getCurrentState().health = 100.f;
        // Reset dying state
        character->getCurrentState().isDying = false;
        character->getCurrentState().isDead = false;

        // Remove weapons
        character->clearInventory();
    }
}

void PlayerController::restartLogic() {
    GameWorld* world = character->engine;
    GameState* state = character->engine->state;

    switch (restartState) {
        case Alive: {
            if (isWasted() || isBusted() || isMissionRestartRequired()) {
                state->fade(2.f, false);

                restartState = FadingOut;
            }

            if (isWasted()) {
                state->setFadeColour(glm::i32vec3(0xc8, 0xc8, 0xc8));

                // Show "wasted" text
                const auto& gxtEntry = "DEAD";
                const auto& text = world->data->texts.text(gxtEntry);
                state->text.addText<ScreenTextType::Big>(
                    ScreenTextEntry::makeBig(gxtEntry, text, 3, 4000));
            }

            if (isBusted()) {
                state->setFadeColour(glm::i32vec3(0x00, 0x00, 0x00));

                // Show "busted" text
                const auto& gxtEntry = "BUSTED";
                const auto& text = world->data->texts.text(gxtEntry);
                state->text.addText<ScreenTextType::Big>(
                    ScreenTextEntry::makeBig(gxtEntry, text, 3, 5000));
            }

            if (isMissionRestartRequired()) {
                state->setFadeColour(glm::i32vec3(0x00, 0x00, 0x00));
            }

            break;
        }
        case FadingOut: {
            if (!state->isFading()) {
                restartState = Restarting;
            }

            break;
        }
        case Restarting: {
            state->fade(4.f, true);

            if (isWasted()) {
                state->setFadeColour(glm::i32vec3(0xc8, 0xc8, 0xc8));
            }

            if (isBusted() || isMissionRestartRequired()) {
                state->setFadeColour(glm::i32vec3(0x00, 0x00, 0x00));
            }

            restart();

            restartState = FadingIn;

            break;
        }
        case FadingIn: {
            if (!state->isFading()) {
                restartState = Alive;
            }

            break;
        }
    }
}

void PlayerController::pickUpPayphone() {
    payphoneState = PayphoneState::PickingUp;

    character->animator->playAnimation(
        AnimIndexMovement, character->animations->animation(AnimCycle::PhoneIn),
        1.f, false);
}

void PlayerController::hangUpPayphone() {
    payphoneState = PayphoneState::HangingUp;

    character->animator->playAnimation(
        AnimIndexMovement,
        character->animations->animation(AnimCycle::PhoneOut), 1.f, false);
}

void PlayerController::talkOnPayphone() {
    payphoneState = PayphoneState::Talking;
}

void PlayerController::leavePayphone() {
    payphoneState = Left;
}

bool PlayerController::isPickingUpPayphone() const {
    return payphoneState == PayphoneState::PickingUp;
}

bool PlayerController::isHangingUpPayphone() const {
    return payphoneState == PayphoneState::HangingUp;
}

bool PlayerController::isTalkingOnPayphone() const {
    return payphoneState == PayphoneState::Talking;
}

void PlayerController::update(float dt) {
    restartLogic();

    GameWorld* world = character->engine;
    GameState* state = character->engine->state;

    if (adrenalineEffect) {
        if (world->getGameTime() > adrenalineEffectTime) {
            state->basic.timeScale = 1.f;
            adrenalineEffect = false;
        }
    }

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

void PlayerController::activateAdrenalineEffect() {
    GameWorld* world = character->engine;
    GameState* state = character->engine->state;

    adrenalineEffect = true;
    adrenalineEffectTime = world->getGameTime() + 20.f;
    state->basic.timeScale = 0.3f;
}

void PlayerController::prepareForCutscene() {
    setInputEnabled(false);

    // @todo: make player invincible
    //                    ignored by police

    // clear all particles, projectiles, extinguish all fires
    //                                              including cars on fire
}

void PlayerController::freeFromCutscene() {
    setInputEnabled(true);

    // @todo: make player no longer invincible
    //                              ignored by police
}