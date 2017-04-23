#include <ai/activity/ExitVehicle.hpp>
#include <engine/Animator.hpp>
#include <objects/VehicleObject.hpp>

bool ExitVehicle::update(CharacterObject *character,
                                     CharacterController *controller) {
    RW_UNUSED(controller);

    if (!character->isInVehicle()) return true;

    auto vehicle = character->getCurrentVehicle();
    auto seat = character->getCurrentSeat();
    auto door = vehicle->getSeatEntryDoor(seat);
    RW_UNUSED(door);
    auto exitPos = vehicle->getSeatEntryPositionWorld(seat);
    auto exitPosLocal = vehicle->getSeatEntryPosition(seat);
    auto cycle_exit = jacked ?
                      (exitPosLocal.x > 0.f ? AnimCycle::CarJackedRHS : AnimCycle::CarJackedLHS) :
                      (exitPosLocal.x > 0.f ? AnimCycle::CarGetOutRHS : AnimCycle::CarGetOutLHS);

    if (vehicle->getVehicle()->vehicletype_ == VehicleModelInfo::BOAT) {
        auto ppos = character->getPosition();
        character->enterVehicle(nullptr, seat);
        character->setPosition(ppos);
        return true;
    }
        // If the vehicle is going too fast, slow down
    else if (!jacked && character->isDriver() && !vehicle->canOccupantExit()) {
        vehicle->setBraking(1.f);
        return false;
    } else if (character->getCurrentCycle() != cycle_exit) {
        character->playCycle(cycle_exit);
        if (door) {
            vehicle->setPartTarget(door, true, door->openAngle);
        }
    } else if (character->animator->isCompleted(AnimIndexAction)) {
        character->enterVehicle(nullptr, seat);
        character->setPosition(exitPos);

        if (character->isDriver()) {
            // Apply the handbrake
            vehicle->setHandbraking(true);
            vehicle->setThrottle(0.f);
        }
        return true;
    }
    return false;
}