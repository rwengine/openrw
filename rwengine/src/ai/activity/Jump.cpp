#include <ai/activity/Jump.hpp>

bool Jump::update(CharacterObject *character,
                              CharacterController *controller) {
    RW_UNUSED(controller);
    if (character->physCharacter == nullptr) return true;

    if (!jumped) {
        character->jump();
        jumped = true;
    } else if (character->physCharacter->canJump()) {
        return true;
    }

    return false;
}