#include <ai/activity/EnterVehicle.hpp>
#include <ai/activity/ExitVehicle.hpp>
#include <engine/Animator.hpp>
#include <objects/VehicleObject.hpp>
#include <BulletDynamics/ConstraintSolver/btHingeConstraint.h>

bool EnterVehicle::canSkip(CharacterObject *character,
                                       CharacterController *) const {
    // If we're already inside the vehicle, it can't helped.
    return !character->isInVehicle();
}

bool EnterVehicle::update(CharacterObject *character,
                                      CharacterController *controller) {
    constexpr float kSprintToEnterDistance = 5.f;
    constexpr float kGiveUpDistance = 100.f;

    RW_UNUSED(controller);

    // Boats don't have any kind of entry animation unless you're onboard.
    if (vehicle->getVehicle()->vehicletype_ == VehicleModelInfo::BOAT) {
        character->enterVehicle(vehicle, seat);
        return true;
    }

    if (seat == ANY_SEAT) {
        // Determine which seat to take.
        float nearest = std::numeric_limits<float>::max();
        for (unsigned int s = 1; s < vehicle->info->seats.size(); ++s) {
            auto entry = vehicle->getSeatEntryPositionWorld(s);
            float dist = glm::distance(entry, character->getPosition());
            if (dist < nearest) {
                seat = s;
                nearest = dist;
            }
        }
    }

    auto entryDoor = vehicle->getSeatEntryDoor(seat);
    auto entryPos = vehicle->getSeatEntryPositionWorld(seat);
    auto entryPosLocal = vehicle->getSeatEntryPosition(seat);

    auto cycle_open = AnimCycle::CarOpenLHS;
    auto cycle_enter = AnimCycle::CarGetInLHS;
    auto cycle_pullout = AnimCycle::CarPullOutLHS;

    if (entryPosLocal.x > 0.f) {
        cycle_open = AnimCycle::CarOpenRHS;
        cycle_enter = AnimCycle::CarGetInRHS;
        cycle_pullout = AnimCycle::CarPullOutRHS;
    }

    // If there's someone in this seat already, we may have to ask them to
    // leave.
    auto currentOccupant =
            static_cast<CharacterObject *>(vehicle->getOccupant(seat));

    bool tryToEnter = false;

    if (entering) {
        if (character->getCurrentCycle() == cycle_open) {
            if (character->animator->isCompleted(AnimIndexAction)) {
                tryToEnter = true;
            } else if (entryDoor &&
                       character->animator->getAnimationTime(AnimIndexAction) >=
                       0.5f) {
                vehicle->setPartTarget(entryDoor, true, entryDoor->openAngle);
            } else {
                // character->setPosition(vehicle->getSeatEntryPosition(seat));
                character->rotation = vehicle->getRotation();
            }
        } else if (character->getCurrentCycle() == cycle_pullout) {
            if (character->animator->isCompleted(AnimIndexAction)) {
                tryToEnter = true;
            }
        } else if (character->getCurrentCycle() == cycle_enter) {
            if (character->animator->isCompleted(AnimIndexAction)) {
                /// @todo move to a more suitable place
                vehicle->grantOccupantRewards(character);

                // VehicleGetIn is over, finish activity
                return true;
            }
        }
    } else {
        glm::vec3 targetDirection = entryPos - character->getPosition();
        targetDirection.z = 0.f;

        float targetDistance = glm::length(targetDirection);

        if (targetDistance <= 0.4f) {
            entering = true;
            // Warp character to vehicle orientation
            character->controller->setMoveDirection({0.f, 0.f, 0.f});
            character->controller->setRunning(false);
            character->setHeading(
                    glm::degrees(glm::roll(vehicle->getRotation())));

            // Determine if the door open animation should be skipped.
            if (entryDoor == nullptr ||
                (entryDoor->constraint != nullptr &&
                 glm::abs(entryDoor->constraint->getHingeAngle()) >= 0.6f)) {
                tryToEnter = true;
            } else {
                character->playCycle(cycle_open);
            }
        } else if (targetDistance > kGiveUpDistance) {
            return true;
        } else {
            if (targetDistance > kSprintToEnterDistance) {
                character->controller->setRunning(true);
            }
            character->setHeading(
                    glm::degrees(atan2(targetDirection.y, targetDirection.x) -
                                 glm::half_pi<float>()));
            character->controller->setMoveDirection({1.f, 0.f, 0.f});
        }
    }

    if (tryToEnter) {
        if (currentOccupant != nullptr && currentOccupant != character) {
            // Play the pullout animation and tell the other character to get
            // out.
            character->playCycle(cycle_pullout);
            currentOccupant->controller->setNextActivity(
                    new ExitVehicle(true));
        } else {
            character->playCycle(cycle_enter);
            character->enterVehicle(vehicle, seat);
        }
    }
    return false;
}