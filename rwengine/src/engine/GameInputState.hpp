#ifndef RWENGINE_GAMEINPUTSTATE_HPP
#define RWENGINE_GAMEINPUTSTATE_HPP

struct GameInputState
{
	static constexpr float kButtonOnThreshold = 0.1f;

	/// Inputs that can be controlled
	/// @todo find any sensible values
	enum Control {
		/* On Foot */
		FireWeapon = 0,
		NextWeapon,
		NextTarget = NextWeapon,
		LastWeapon,
		LastTarget = LastWeapon,
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
		VehicleFireWeapon = FireWeapon,
		VehicleAccelerate,
		VehicleBrake,
		VehicleLeft = GoLeft,
		VehicleRight = GoRight,
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

		_MaxControls
	};

	/**
	 * Current state of each control [0 to 1].
	 *
	 * For buttons, this will result in either 0 or 1.
	 */
	float currentLevels[_MaxControls] = { };

	float operator[] (Control c) const
	{
		return currentLevels[c];
	}

	bool pressed(Control c) const
	{
		return currentLevels[c] > kButtonOnThreshold;
	}
};

#endif
