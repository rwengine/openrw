#include "GarageController.hpp"

#include <btBulletDynamicsCommon.h>
#include <glm/gtx/quaternion.hpp>

#include "dynamics/CollisionInstance.hpp"

#include "ai/PlayerController.hpp"

#include "engine/GameState.hpp"

#include "objects/CharacterObject.hpp"
#include "objects/GameObject.hpp"
#include "objects/InstanceObject.hpp"
#include "objects/VehicleObject.hpp"

GarageController::GarageController(GameWorld* engine, GarageInfo* info,
                                   InstanceObject* door)

    : swingType(false)
    , fraction(0.f)
    , step(0.5f)
    , doorHeight(4.0f)
    , engine(engine)
    , garageInfo(info)
    , doorObject(door) {
    if (doorObject) {
        startPosition = doorObject->getPosition();
        startRotation = doorObject->getRotation();
    }
    if (garageInfo) {
        if (garageInfo->state == GarageState::Closed) {
            fraction = 0.f;
        } else {
            fraction = 1.f;
        }
    }

    // @todo set door height according to model size
}

GarageController::~GarageController() {
}

float GarageController::getDistanceToGarage(glm::vec3 point) {
    float dx = std::max(
        {garageInfo->min.x - point.x, 0.f, point.x - garageInfo->max.x});
    float dy = std::max(
        {garageInfo->min.y - point.y, 0.f, point.y - garageInfo->max.y});

    // Seems like original game ignores z axis, bug or feature?
    // float dz = std::max(
    //    {garageInfo->min.z - point.z, 0.f, point.z - garageInfo->max.z});

    // return std::sqrt(dx * dx + dy * dy + dz * dz);

    return std::sqrt(dx * dx + dy * dy);
}

bool GarageController::isObjectInsideGarage(GameObject* object) {
    // This is not that trivial, we need to check if full vehicle body is inside
    auto p = object->getPosition();

    if (p.x >= (garageInfo->min.x) && p.y >= (garageInfo->min.y) &&
        p.z >= (garageInfo->min.z) && p.x <= (garageInfo->max.x) &&
        p.y <= (garageInfo->max.y) && p.z <= (garageInfo->max.z)) {
        return true;
    }

    return false;
}

bool GarageController::shouldClose() {
    auto controller = engine->players.at(0);
    auto plyChar = controller->getCharacter();
    auto playerPosition = plyChar->getPosition();
    auto playerVehicle = plyChar->getCurrentVehicle();
    bool playerIsInVehicle = playerVehicle != nullptr;

    switch (garageInfo->type) {
        case GarageType::Mission: {
            if (!isObjectInsideGarage(static_cast<GameObject*>(plyChar)) &&
                isObjectInsideGarage(garageInfo->target)) {
                return true;
            }

            return false;
        }

        case GarageType::BombShop1:
        case GarageType::BombShop2:
        case GarageType::BombShop3:
        case GarageType::Respray: {
            if (playerIsInVehicle) {
                if (isObjectInsideGarage(
                        static_cast<GameObject*>(playerVehicle)) &&
                    playerVehicle->isStopped()) {
                    return true;
                }
            }

            return false;
        }

        case GarageType::CollectCars1:
        case GarageType::CollectCars2: {
            if (playerIsInVehicle) {
                if (isObjectInsideGarage(
                        static_cast<GameObject*>(playerVehicle))) {
                    if (playerVehicle->getLifetime() !=
                        GameObject::MissionLifetime) {
                        return true;
                    } else {
                        // @todo show message "come back when youre not busy"
                    }
                }
            }

            return false;
        }

        case GarageType::MissionForCarToComeOut: {
            // @todo unimplemented
            return false;
        }

        case GarageType::Crusher: {
            // @todo unimplemented
            return false;
        }

        case GarageType::MissionKeepCar: {
            // @todo unimplemented
            return false;
        }

        case GarageType::Hideout1:
        case GarageType::Hideout2:
        case GarageType::Hideout3: {
            // Not sure about these values
            if ((!playerIsInVehicle &&
                 getDistanceToGarage(playerPosition) >= 5.f) ||
                (playerIsInVehicle &&
                 getDistanceToGarage(playerPosition) >= 10.f)) {
                return true;
            }

            return false;
        }

        case GarageType::MissionToOpenAndClose: {
            // @todo unimplemented
            return false;
        }

        case GarageType::MissionForSpecificCar: {
            // @todo unimplemented
            return false;
        }

        case GarageType::MissionKeepCarAndRemainClosed: {
            // @todo unimplemented
            return false;
        }
    }

    return false;
}

bool GarageController::shouldOpen() {
    auto controller = engine->players.at(0);
    auto plyChar = controller->getCharacter();
    auto playerPosition = plyChar->getPosition();
    auto playerVehicle = plyChar->getCurrentVehicle();
    bool playerIsInVehicle = playerVehicle != nullptr;

    switch (garageInfo->type) {
        case GarageType::Mission: {
            // Not sure about these values
            if (playerIsInVehicle &&
                getDistanceToGarage(playerPosition) < 8.f &&
                playerVehicle == garageInfo->target) {
                return true;
            }

            return false;
        }

        case GarageType::BombShop1:
        case GarageType::BombShop2:
        case GarageType::BombShop3:
        case GarageType::Respray: {
            if (garageTimer < engine->getGameTime()) {
                return true;
            }

            return false;
        }

        case GarageType::CollectCars1:
        case GarageType::CollectCars2: {
            // @todo unimplemented
            return false;
        }

        case GarageType::MissionForCarToComeOut: {
            // @todo unimplemented
            return false;
        }

        case GarageType::Crusher: {
            // @todo unimplemented
            return false;
        }

        case GarageType::MissionKeepCar: {
            // @todo unimplemented
            return false;
        }

        case GarageType::Hideout1:
        case GarageType::Hideout2:
        case GarageType::Hideout3: {
            // Not sure about these values
            if ((!playerIsInVehicle &&
                 getDistanceToGarage(playerPosition) < 5.f) ||
                (playerIsInVehicle &&
                 getDistanceToGarage(playerPosition) < 10.f)) {
                return true;
            }

            return false;
        }

        case GarageType::MissionToOpenAndClose: {
            // @todo unimplemented
            return false;
        }

        case GarageType::MissionForSpecificCar: {
            // @todo unimplemented
            return false;
        }

        case GarageType::MissionKeepCarAndRemainClosed: {
            // @todo unimplemented
            return false;
        }
    }

    return false;
}

bool GarageController::shouldStopClosing() {
    auto controller = engine->players.at(0);
    auto plyChar = controller->getCharacter();
    auto playerPosition = plyChar->getPosition();
    auto playerVehicle = plyChar->getCurrentVehicle();
    bool playerIsInVehicle = playerVehicle != nullptr;

    switch (garageInfo->type) {
        case GarageType::Mission:
        case GarageType::BombShop1:
        case GarageType::BombShop2:
        case GarageType::BombShop3:
        case GarageType::Respray:
        case GarageType::CollectCars1:
        case GarageType::CollectCars2: {
            return false;
        }

        case GarageType::MissionForCarToComeOut: {
            // @todo unimplemented
            return false;
        }

        case GarageType::Crusher: {
            // @todo unimplemented
            return false;
        }

        case GarageType::MissionKeepCar: {
            // @todo unimplemented
            return false;
        }

        case GarageType::Hideout1:
        case GarageType::Hideout2:
        case GarageType::Hideout3: {
            // Not sure about these values
            if ((!playerIsInVehicle &&
                 getDistanceToGarage(playerPosition) < 5.f) ||
                (playerIsInVehicle &&
                 getDistanceToGarage(playerPosition) < 10.f)) {
                return true;
            }

            return false;
        }

        case GarageType::MissionToOpenAndClose: {
            // @todo unimplemented
            return false;
        }

        case GarageType::MissionForSpecificCar: {
            // @todo unimplemented
            return false;
        }

        case GarageType::MissionKeepCarAndRemainClosed: {
            // @todo unimplemented
            return false;
        }
    }

    return false;
}

bool GarageController::shouldStopOpening() {
    auto controller = engine->players.at(0);
    auto plyChar = controller->getCharacter();
    auto playerPosition = plyChar->getPosition();
    auto playerVehicle = plyChar->getCurrentVehicle();
    bool playerIsInVehicle = playerVehicle != nullptr;

    switch (garageInfo->type) {
        case GarageType::Mission:
        case GarageType::BombShop1:
        case GarageType::BombShop2:
        case GarageType::BombShop3:
        case GarageType::Respray:
        case GarageType::CollectCars1:
        case GarageType::CollectCars2: {
            return false;
        }

        case GarageType::MissionForCarToComeOut: {
            // @todo unimplemented
            return false;
        }

        case GarageType::Crusher: {
            // @todo unimplemented
            return false;
        }

        case GarageType::MissionKeepCar: {
            // @todo unimplemented
            return false;
        }

        case GarageType::Hideout1:
        case GarageType::Hideout2:
        case GarageType::Hideout3: {
            // Not sure about these values
            if ((!playerIsInVehicle &&
                 getDistanceToGarage(playerPosition) >= 5.f) ||
                (playerIsInVehicle &&
                 getDistanceToGarage(playerPosition) >= 10.f)) {
                return true;
            }

            return false;
        }

        case GarageType::MissionToOpenAndClose: {
            // @todo unimplemented
            return false;
        }

        case GarageType::MissionForSpecificCar: {
            // @todo unimplemented
            return false;
        }

        case GarageType::MissionKeepCarAndRemainClosed: {
            // @todo unimplemented
            return false;
        }
    }

    return false;
}

void GarageController::doOnOpenEvent() {
    auto controller = engine->players.at(0);
    auto plyChar = controller->getCharacter();
    auto playerPosition = plyChar->getPosition();
    auto playerVehicle = plyChar->getCurrentVehicle();
    bool playerIsInVehicle = playerVehicle != nullptr;

    switch (garageInfo->type) {
        case GarageType::Mission: {
            break;
        }

        case GarageType::BombShop1: {
            break;
        }

        case GarageType::BombShop2: {
            break;
        }

        case GarageType::BombShop3: {
            break;
        }

        case GarageType::Respray: {
            break;
        }

        case GarageType::CollectCars1:
        case GarageType::CollectCars2: {
            break;
        }

        case GarageType::MissionForCarToComeOut: {
            break;
        }

        case GarageType::Crusher: {
            break;
        }

        case GarageType::MissionKeepCar: {
            break;
        }

        case GarageType::Hideout1:
        case GarageType::Hideout2:
        case GarageType::Hideout3: {
            break;
        }

        case GarageType::MissionToOpenAndClose: {
            break;
        }

        case GarageType::MissionForSpecificCar: {
            break;
        }

        case GarageType::MissionKeepCarAndRemainClosed: {
            break;
        }
    }
}

void GarageController::doOnCloseEvent() {
    auto controller = engine->players.at(0);
    auto plyChar = controller->getCharacter();
    auto playerPosition = plyChar->getPosition();
    auto playerVehicle = plyChar->getCurrentVehicle();
    bool playerIsInVehicle = playerVehicle != nullptr;

    switch (garageInfo->type) {
        case GarageType::Mission: {
            break;
        }

        case GarageType::BombShop1:
        case GarageType::BombShop2:
        case GarageType::BombShop3: {
            // Find out real value
            garageTimer = engine->getGameTime() + 1.5f;

            break;
        }

        case GarageType::Respray: {
            // Find out real value
            garageTimer = engine->getGameTime() + 2.f;
            playerVehicle->setHealth(1000.f);

            break;
        }

        case GarageType::CollectCars1:
        case GarageType::CollectCars2: {
            break;
        }

        case GarageType::MissionForCarToComeOut: {
            break;
        }

        case GarageType::Crusher: {
            break;
        }

        case GarageType::MissionKeepCar: {
            break;
        }

        case GarageType::Hideout1:
        case GarageType::Hideout2:
        case GarageType::Hideout3: {
            break;
        }

        case GarageType::MissionToOpenAndClose: {
            break;
        }

        case GarageType::MissionForSpecificCar: {
            break;
        }

        case GarageType::MissionKeepCarAndRemainClosed: {
            break;
        }
    }
}

void GarageController::doOnStartOpeningEvent() {
    auto controller = engine->players.at(0);
    auto plyChar = controller->getCharacter();
    auto playerPosition = plyChar->getPosition();
    auto playerVehicle = plyChar->getCurrentVehicle();
    bool playerIsInVehicle = playerVehicle != nullptr;

    switch (garageInfo->type) {
        case GarageType::Mission:
        case GarageType::CollectCars1:
        case GarageType::CollectCars2: {
            controller->setInputEnabled(true);
            break;
        }

        case GarageType::BombShop1:
        case GarageType::BombShop2:
        case GarageType::BombShop3:
        case GarageType::Respray: {
            controller->setInputEnabled(true);
            playerVehicle->setHandbraking(false);
            break;
        }

        case GarageType::MissionForCarToComeOut: {
            break;
        }

        case GarageType::Crusher: {
            break;
        }

        case GarageType::MissionKeepCar: {
            break;
        }

        case GarageType::Hideout1:
        case GarageType::Hideout2:
        case GarageType::Hideout3: {
            break;
        }

        case GarageType::MissionToOpenAndClose: {
            break;
        }

        case GarageType::MissionForSpecificCar: {
            break;
        }

        case GarageType::MissionKeepCarAndRemainClosed: {
            break;
        }
    }
}

void GarageController::doOnStartClosingEvent() {
    auto controller = engine->players.at(0);
    auto plyChar = controller->getCharacter();
    auto playerPosition = plyChar->getPosition();
    auto playerVehicle = plyChar->getCurrentVehicle();
    bool playerIsInVehicle = playerVehicle != nullptr;

    switch (garageInfo->type) {
        case GarageType::Mission:
        case GarageType::CollectCars1:
        case GarageType::CollectCars2: {
            controller->setInputEnabled(false);
            break;
        }

        case GarageType::BombShop1:
        case GarageType::BombShop2:
        case GarageType::BombShop3:
        case GarageType::Respray: {
            controller->setInputEnabled(false);
            playerVehicle->setHandbraking(true);
            break;
        }

        case GarageType::MissionForCarToComeOut: {
            break;
        }

        case GarageType::Crusher: {
            break;
        }

        case GarageType::MissionKeepCar: {
            break;
        }

        case GarageType::MissionToOpenAndClose: {
            break;
        }

        case GarageType::MissionForSpecificCar: {
            break;
        }

        case GarageType::MissionKeepCarAndRemainClosed: {
            break;
        }
    }
}

void GarageController::tick(float dt) {
    if (!garageInfo) return;
    if (!doorObject) return;

    switch (garageInfo->state) {
        case GarageState::Opened: {
            if (shouldClose()) {
                garageInfo->state = GarageState::Closing;
                doOnStartClosingEvent();
            }

            break;
        }

        case GarageState::Closed: {
            if (shouldOpen()) {
                garageInfo->state = GarageState::Opening;
                doOnStartOpeningEvent();
            }

            break;
        }

        case GarageState::Opening: {
            if (shouldStopOpening()) {
                garageInfo->state = GarageState::Closing;
            } else {
                fraction += dt * step;

                if (fraction >= 1.0f) {
                    garageInfo->state = GarageState::Opened;
                    fraction = 1.f;
                    doOnOpenEvent();
                }
            }

            break;
        }

        case GarageState::Closing: {
            if (shouldStopClosing()) {
                garageInfo->state = GarageState::Opening;
            } else {
                fraction -= dt * step;

                if (fraction <= 0.f) {
                    garageInfo->state = GarageState::Closed;
                    fraction = 0.f;
                    doOnCloseEvent();
                }
            }

            break;
        }
    }

    if (swingType) {
        // @todo incomplete
        doorObject->setRotation(
            glm::angleAxis(fraction * 1.57079632679f, glm::vec3(0, 1, 0)));
    } else {
        doorObject->setPosition(startPosition +
                                glm::vec3(0.f, 0.f, fraction * doorHeight));
    }
}