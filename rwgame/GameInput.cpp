#include "GameInput.hpp"
#include <unordered_map>

// Hardcoded Controls SDLK_* -> GameInputState::Control
const std::unordered_multimap<int, GameInputState::Control> kDefaultControls = {
    /* On Foot */
    {SDLK_LCTRL, GameInputState::FireWeapon},
    {SDLK_KP_0, GameInputState::FireWeapon},
    {SDLK_KP_ENTER, GameInputState::NextWeapon},
    {SDLK_KP_PERIOD, GameInputState::LastWeapon},
    {SDLK_w, GameInputState::GoForward},
    {SDLK_UP, GameInputState::GoForward},
    {SDLK_s, GameInputState::GoBackwards},
    {SDLK_DOWN, GameInputState::GoBackwards},
    {SDLK_a, GameInputState::GoLeft},
    {SDLK_LEFT, GameInputState::GoLeft},
    {SDLK_d, GameInputState::GoRight},
    {SDLK_RIGHT, GameInputState::GoRight},
    {SDLK_PAGEUP, GameInputState::ZoomIn},
    {SDLK_z, GameInputState::ZoomIn},
    {SDLK_PAGEDOWN, GameInputState::ZoomOut},
    {SDLK_x, GameInputState::ZoomOut},
    {SDLK_f, GameInputState::EnterExitVehicle},
    {SDLK_RETURN, GameInputState::EnterExitVehicle},
    {SDLK_c, GameInputState::ChangeCamera},
    {SDLK_HOME, GameInputState::ChangeCamera},
    {SDLK_RCTRL, GameInputState::Jump},
    {SDLK_SPACE, GameInputState::Jump},
    {SDLK_LSHIFT, GameInputState::Sprint},
    {SDLK_RSHIFT, GameInputState::Sprint},
    {SDLK_LALT, GameInputState::Walk},
    {SDLK_DELETE, GameInputState::AimWeapon},
    {SDLK_CAPSLOCK, GameInputState::LookBehind},

    /* In Vehicle */
    {SDLK_LCTRL, GameInputState::VehicleFireWeapon},
    {SDLK_a, GameInputState::VehicleLeft},
    {SDLK_LEFT, GameInputState::VehicleLeft},
    {SDLK_d, GameInputState::VehicleRight},
    {SDLK_RIGHT, GameInputState::VehicleRight},
    {SDLK_w, GameInputState::VehicleAccelerate},
    {SDLK_UP, GameInputState::VehicleAccelerate},
    {SDLK_d, GameInputState::VehicleBrake},
    {SDLK_DOWN, GameInputState::VehicleBrake},
    {SDLK_INSERT, GameInputState::ChangeRadio},
    {SDLK_r, GameInputState::ChangeRadio},
    {SDLK_LSHIFT, GameInputState::Horn},
    {SDLK_RSHIFT, GameInputState::Horn},
    {SDLK_KP_PLUS, GameInputState::Submission},
    {SDLK_CAPSLOCK, GameInputState::Submission},
    {SDLK_RCTRL, GameInputState::Handbrake},
    {SDLK_SPACE, GameInputState::Handbrake},
    {SDLK_KP_9, GameInputState::VehicleAimUp},
    {SDLK_KP_2, GameInputState::VehicleAimDown},
    {SDLK_KP_4, GameInputState::VehicleAimLeft},
    {SDLK_KP_6, GameInputState::VehicleAimRight},
    {SDLK_KP_9, GameInputState::VehicleDown},
    {SDLK_KP_2, GameInputState::VehicleUp},
    {SDLK_KP_1, GameInputState::LookLeft},
    {SDLK_q, GameInputState::LookLeft},
    {SDLK_KP_2, GameInputState::LookRight},
    {SDLK_e, GameInputState::LookRight},
};

void GameInput::updateGameInputState(GameInputState *state,
                                     const SDL_Event &event) {
    switch (event.type) {
        case SDL_KEYDOWN:
        case SDL_KEYUP: {
            auto sym = event.key.keysym.sym;
            auto level = event.type == SDL_KEYDOWN ? 1.f : 0.f;
            auto& levels = state->levels;

            auto [rangeBegin, rangeEnd] = kDefaultControls.equal_range(sym);
            for (auto it = rangeBegin; it != rangeEnd; ++it) {
                levels[it->second] = level;
            }
        } break;
    }
}
