#ifndef RWENGINE_GAMEINPUTSTATE_HPP
#define RWENGINE_GAMEINPUTSTATE_HPP

struct GameInputState {
    static constexpr float kButtonOnThreshold = 0.1f;

    /// Inputs that can be controlled
    /// @todo find any sensible values
    enum Control {
        /* On Foot */
        FireWeapon = 0,
        NextWeapon,
        LastWeapon,
        GoForward,
        GoBackwards,
        GoLeft,
        GoRight,
        ZoomIn,
        ZoomOut,
        EnterExitVehicle,
        ChangeCamera,
        Jump,
        Sprint,
        AimWeapon,
        Crouch,
        Walk,
        LookBehind,

        /* In Vehicle */
        VehicleAccelerate,
        VehicleBrake,
        VehicleDown,
        VehicleUp,
        ChangeRadio,
        Horn,
        Submission,
        Handbrake,
        LookLeft,
        LookRight,
        VehicleAimLeft,
        VehicleAimRight,
        VehicleAimUp,
        VehicleAimDown,

        _MaxControls,

        NextTarget = NextWeapon,
        LastTarget = LastWeapon,
        VehicleLeft = GoLeft,
        VehicleRight = GoRight,
        VehicleFireWeapon = FireWeapon,
    };

    /**
     * Current state of each control [0 to 1].
     *
     * For buttons, this will result in either 0 or 1.
     */
    float levels[_MaxControls] = {};

    float operator[](Control c) const {
        return levels[c];
    }

    /**
     * @return if cotrol is held down
     */
    bool pressed(Control c) const {
        return levels[c] > kButtonOnThreshold;
    }
};

#endif
