#pragma once
#ifndef _EXITVEHICLE_HPP_
#define _EXITVEHICLE_HPP_
#include <ai/activity/Activity.hpp>

struct ExitVehicle : public CharacterController::Activity {
    DECL_ACTIVITY(ExitVehicle)

            const bool jacked;

    ExitVehicle(bool jacked_ = false) : jacked(jacked_) {
    }

    bool update(CharacterObject* character, CharacterController* controller);
};

#endif
