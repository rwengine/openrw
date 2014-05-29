#pragma once
#ifndef _GTAAICONTROLLER_HPP_
#define _GTAAICONTROLLER_HPP_
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct GTACharacter;
/**
 * @class GTAAIController
 * Character Controller Interface, translates high-level behaviours into low level actions.
 */
class GTAAIController
{
public:

	enum Activity {
		Idle,
		GoTo,
	};

	struct ActivityParameter {
		glm::vec3 position;

		ActivityParameter( ) {}

		ActivityParameter( const glm::vec3& position ) :
			position( position ) {}
	};

protected:
	
	/**
	 * The character being controlled.
	 */
	GTACharacter* character;

	Activity _currentActivity;
	Activity _nextActivity;

	ActivityParameter _currentParameter;
	ActivityParameter _nextParameter;

	bool updateActivity();
	void setActivity(Activity activity, const ActivityParameter& _parameter );

public:
	
	GTAAIController(GTACharacter* character);

	Activity getCurrentActivity() const { return _currentActivity; }
	const ActivityParameter& getCurrentActivityParameter() const { return _currentParameter; }

	Activity getNextActivity() const { return _nextActivity; }
	const ActivityParameter& getNextActivityParameter() const { return _nextParameter; }

	/**
	 * @brief setNextActivity Sets the next Activity with a parameter.
	 * @param activity
	 * @param position
	 */
	void setNextActivity( Activity activity, const ActivityParameter& parameter );
	
	/**
	 * @brief update Updates the controller.
	 * @param dt
	 */
	virtual void update(float dt);

	virtual glm::vec3 getTargetPosition() = 0;
};

#endif
