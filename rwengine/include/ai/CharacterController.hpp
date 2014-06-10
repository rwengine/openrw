#pragma once
#ifndef _CHARACTERCONTROLLER_HPP_
#define _CHARACTERCONTROLLER_HPP_
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>

struct CharacterObject;
struct VehicleObject;

/**
 * @class CharacterController
 * Character Controller Interface, translates high-level behaviours into low level actions.
 */
class CharacterController
{
public:

	/**
	 * @brief The Activity struct interface
	 */
	struct Activity {

		virtual ~Activity() {}

		virtual std::string name() const = 0;

		virtual bool update(CharacterObject* character, CharacterController* controller) = 0;
	};

protected:
	
	/**
	 * The character being controlled.
	 */
	CharacterObject* character;

	Activity* _currentActivity;
	Activity* _nextActivity;

	bool updateActivity();
	void setActivity(Activity* activity);

public:
	
	CharacterController(CharacterObject* character);

	virtual ~CharacterController() { }

	Activity* getCurrentActivity() { return _currentActivity; }

	Activity* getNextActivity() { return _nextActivity; }

	/**
	 * @brief skipActivity Cancel the current activity, immediatley.
	 */
	void skipActivity();

	/**
	 * @brief setNextActivity Sets the next Activity with a parameter.
	 * @param activity
	 * @param position
	 */
	void setNextActivity( Activity* activity );
	
	/**
	 * @brief update Updates the controller.
	 * @param dt
	 */
	virtual void update(float dt);

	virtual glm::vec3 getTargetPosition() = 0;
};

#define DECL_ACTIVITY( activity_name ) \
	std::string name() const { return #activity_name; }

// TODO: Refactor this with an ugly macro to reduce code dup.

namespace Activities {
	struct GoTo : public CharacterController::Activity {
		DECL_ACTIVITY( GoTo )

		glm::vec3 target;

		GoTo( const glm::vec3& target )
			: target( target ) {}

		bool update(CharacterObject* character, CharacterController* controller);
	};

	struct EnterVehicle : public CharacterController::Activity {
		DECL_ACTIVITY( EnterVehicle )

		VehicleObject* vehicle;
		unsigned int seat;

		bool entering;

		EnterVehicle( VehicleObject* vehicle, unsigned int seat = 0 )
			: vehicle( vehicle ), seat( seat ), entering(false) {}

		bool update(CharacterObject *character, CharacterController *controller);
	};

	struct ExitVehicle : public CharacterController::Activity {
		DECL_ACTIVITY( ExitVehicle )

		ExitVehicle( )
			{}

		bool update(CharacterObject *character, CharacterController *controller);
	};
}

#endif
