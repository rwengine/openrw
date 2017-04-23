#pragma once
#ifndef _JUMP_HPP_
#define _JUMP_HPP_
#include <ai/activity/Activity.hpp>

struct Jump : public CharacterController::Activity {
    DECL_ACTIVITY(Jump)

    bool jumped;

    Jump() : jumped(false) {
    }

    bool update(CharacterObject* character, CharacterController* controller);
};

#endif
