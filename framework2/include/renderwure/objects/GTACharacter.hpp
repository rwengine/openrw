#pragma once
#ifndef _GTACHARACTER_HPP_
#define _GTACHARACTER_HPP_
#include <renderwure/engine/GTAObject.hpp>
#include <bullet/BulletDynamics/Character/btKinematicCharacterController.h>
#include <bullet/btBulletCollisionCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

class GTAVehicle;

/**
 * @brief The GTACharacter struct
 * Stores data relating to an instance of a "pedestrian".
 */
struct GTACharacter : public GTAObject
{
private:
	GTAVehicle* currentVehicle;

	void createActor();
	void destroyActor();

public:

	enum Activity {
		None,
		Idle,
		Walk,
		Run,
		Crouch,
		VehicleDrive,
		VehicleSit
	};

	std::shared_ptr<LoaderIDE::PEDS_t> ped;

	btKinematicCharacterController* physCharacter;
	btPairCachingGhostObject* physObject;
	btBoxShape* physShape;

	GTAAIController* controller;

	/**
	 * @brief GTACharacter Constructs a Character
	 * @param pos
	 * @param rot
	 * @param model
	 * @param ped PEDS_t struct to use.
	 */
	GTACharacter(GTAEngine* engine, const glm::vec3& pos, const glm::quat& rot, Model* model, std::shared_ptr<LoaderIDE::PEDS_t> ped);

	~GTACharacter();

	Type type() { return Character; }

	Activity currentActivity;

	void changeAction(Activity newAction);

	void tick(float dt);

	/**
	 * @brief updateCharacter updates internall bullet Character.
	 */
	void updateCharacter();

	virtual void setPosition(const glm::vec3& pos);

	virtual glm::vec3 getPosition() const;

	GTAVehicle *getCurrentVehicle() const;
	void setCurrentVehicle(GTAVehicle *value);
};

#endif
