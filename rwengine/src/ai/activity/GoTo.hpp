#pragma once
#ifndef _GOTO_HPP_
#define _GOTO_HPP_
#include <ai/activity/Activity.hpp>

struct GoTo : public CharacterController::Activity {
    DECL_ACTIVITY(GoTo)

    glm::vec3 target;
    bool sprint;

    GoTo(const glm::vec3& target, bool _sprint = false)
            : target(target), sprint(_sprint) {
    }

    bool update(CharacterObject* character, CharacterController* controller);

    bool canSkip(CharacterObject*, CharacterController*) const {
        return true;
    }
};

#endif
