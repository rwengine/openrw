#pragma once
#ifndef _USEITEM_HPP_
#define _USEITEM_HPP_
#include <ai/activity/Activity.hpp>

struct UseItem : public CharacterController::Activity {
    DECL_ACTIVITY(UseItem)

    int itemslot;
    bool fired = false;
    float power = 0.f;

    UseItem(int slot) : itemslot(slot) {
    }

    bool update(CharacterObject* character, CharacterController* controller);
};

#endif
