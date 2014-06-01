#pragma once
#ifndef _GTAAICONTROLLER_HPP_
#define _GTAAICONTROLLER_HPP_
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>

struct GTACharacter;
struct GTAVehicle;

/**
 * @class GTAAIController
 * Character Controller Interface, translates high-level behaviours into low level actions.
 */
class GTAAIController
{
public:

	/**
	 * @brief The Activity struct interface
	 */
	struct Activity {

		virtual ~Activity() {}

		virtual std::string name() const = 0;

		virtual bool update(GTACharacter* character, GTAAIController* controller) = 0;
	};

protected:
	
	/**
	 * The character being controlled.
	 */
	GTACharacter* character;

	Activity* _currentActivity;
	Activity* _nextActivity;

	bool updateActivity();
	void setActivity(Activity* activity);

public:
	
	GTAAIController(GTACharacter* character);

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
	struct GoTo : public GTAAIController::Activity {
		DECL_ACTIVITY( GoTo )

		glm::vec3 target;

		GoTo( const glm::vec3& target )
			: target( target ) {}

		bool update(GTACharacter* character, GTAAIController* controller);
	};

	struct EnterVehicle : public GTAAIController::Activity {
		DECL_ACTIVITY( EnterVehicle )

		GTAVehicle* vehicle;
		unsigned int seat;

		bool entering;

		EnterVehicle( GTAVehicle* vehicle, unsigned int seat = 0 )
			: vehicle( vehicle ), seat( seat ), entering(false) {}

		bool update(GTACharacter *character, GTAAIController *controller);
	};
}

#endif
