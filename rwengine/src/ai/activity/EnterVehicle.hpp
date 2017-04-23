#pragma once
#ifndef _ENTERVEHICLE_HPP_
#define _ENTERVEHICLE_HPP_
#include <ai/activity/Activity.hpp>


struct EnterVehicle : public CharacterController::Activity {
    DECL_ACTIVITY(EnterVehicle)

    VehicleObject* vehicle;
    int seat;

    enum {
        ANY_SEAT = -1  // Magic number for any seat but the driver's.
    };

    bool entering;

    EnterVehicle(VehicleObject* vehicle, int seat = 0)
            : vehicle(vehicle), seat(seat), entering(false) {
    }

    bool canSkip(CharacterObject* character,
                 CharacterController*) const override;

    bool update(CharacterObject* character, CharacterController* controller);
};

#endif
