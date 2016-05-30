#pragma once
#ifndef _CHARACTERCONTROLLER_HPP_
#define _CHARACTERCONTROLLER_HPP_
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>

struct AIGraphNode;
class CharacterObject;
class VehicleObject;

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

		/**
		 * @brief canSkip
		 * @return true if the activity can be skipped.
		 */
		virtual bool canSkip(CharacterObject*, CharacterController*) const { return false; }

		virtual bool update(CharacterObject* character, CharacterController* controller) = 0;
	};
	
	/**
	 * Available AI goals.
	 */
	enum Goal
	{
		/**
		 * No goal, will idle or execute external Activities.
		 */
		None,
		/**
		 * Keep close to leader character
		 */
		FollowLeader,
		/**
		 * Wander randomly around the map
		 */
		TrafficWander
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

	float vehicleIdle;
	
	// Goal related variables
	Goal currentGoal;
	CharacterObject* leader;
	AIGraphNode* targetNode;

public:
	
	CharacterController(CharacterObject* character);

	virtual ~CharacterController() { }

	Activity* getCurrentActivity() const { return _currentActivity; }

	Activity* getNextActivity() const { return _nextActivity; }

	/**
	 * @brief skipActivity Cancel the current activity immediatley, if possible.
	 */
	void skipActivity();

	/**
	 * @brief setNextActivity Sets the next Activity with a parameter.
	 * @param activity
	 * @param position
	 */
	void setNextActivity( Activity* activity );

	/**
	 * @brief IsCurrentActivity
	 * @param activity Name of activity to check for
	 * @return if the given activity is the current activity
	 */
	bool isCurrentActivity(const std::string& activity) const;
	
	/**
	 * @brief update Updates the controller.
	 * @param dt
	 */
	virtual void update(float dt);

	virtual glm::vec3 getTargetPosition() = 0;

	/**
	 * @brief
	 * @return Returns the Character Object
	 */
	CharacterObject* getCharacter() const;

	void setMoveDirection(const glm::vec3& movement);
	void setLookDirection(const glm::vec2& look);

	void setRunning(bool run);
	
	void setGoal(Goal goal) { currentGoal = goal; }
	Goal getGoal() const { return currentGoal; }
	
	void setTargetCharacter(CharacterObject* c) { leader = c; }
	CharacterObject* getTargetCharacter() const { return leader; }
};

#define DECL_ACTIVITY( activity_name ) \
	static constexpr auto ActivityName = #activity_name; \
	std::string name() const { return ActivityName; }

// TODO: Refactor this with an ugly macro to reduce code dup.
class WeaponItem;

/**
 * @brief Activities for CharacterController behaviour
 *
 * @todo Move into ControllerActivities.hpp or equivelant
 */
namespace Activities {
	struct GoTo : public CharacterController::Activity {
		DECL_ACTIVITY( GoTo )

		glm::vec3 target;
		bool sprint;

		GoTo( const glm::vec3& target, bool _sprint = false )
			: target( target ), sprint(_sprint) {}

		bool update(CharacterObject* character, CharacterController* controller);

		bool canSkip(CharacterObject *, CharacterController *) const { return true; }
	};
	
	struct Jump : public CharacterController::Activity
	{
		DECL_ACTIVITY( Jump )
		
		bool jumped;
		
		Jump() : jumped(false) {}
		
		bool update(CharacterObject* character, CharacterController* controller);
	};

	struct EnterVehicle : public CharacterController::Activity {
		DECL_ACTIVITY( EnterVehicle )

		VehicleObject* vehicle;
		int seat;
		
		enum {
			ANY_SEAT = -1 // Magic number for any seat but the driver's.
		};

		bool entering;

		EnterVehicle( VehicleObject* vehicle, int seat = 0 )
			: vehicle( vehicle ), seat( seat ), entering(false) {}

		bool canSkip(CharacterObject* character, CharacterController*) const override;

		bool update(CharacterObject *character, CharacterController *controller);
	};

	struct ExitVehicle : public CharacterController::Activity {
		DECL_ACTIVITY( ExitVehicle )

		ExitVehicle( )
			{}

		bool update(CharacterObject *character, CharacterController *controller);
	};

	struct ShootWeapon : public CharacterController::Activity {
		DECL_ACTIVITY( ShootWeapon )

		WeaponItem* _item;
		bool _fired;

		ShootWeapon( WeaponItem* item )
			: _item(item), _fired(false) {}

		bool update(CharacterObject *character, CharacterController *controller);
	};
}

#endif
