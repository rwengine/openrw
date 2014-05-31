#pragma once
#ifndef _GTACHARACTER_HPP_
#define _GTACHARACTER_HPP_
#include <engine/GameObject.hpp>
#include <bullet/BulletDynamics/Character/btKinematicCharacterController.h>
#include <bullet/btBulletCollisionCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <glm/glm.hpp>

class GTAVehicle;
class GameWorld;

struct AnimationGroup
{
	Animation* idle;
	Animation* walk;
	Animation* walk_start;
	Animation* run;

	Animation* jump_start;
	Animation* jump_glide;
	Animation* jump_land;

	Animation* car_sit;
	Animation* car_sit_low;

	Animation* car_getin_lhs;

	AnimationGroup()
	 : idle(nullptr), walk(nullptr), walk_start(nullptr), run(nullptr),
	   jump_start(nullptr), jump_glide(nullptr), jump_land(nullptr),
	   car_sit(nullptr), car_sit_low(nullptr), car_getin_lhs(nullptr)
	{}
};

/**
 * @brief The GTACharacter struct
 * Stores data relating to an instance of a "pedestrian".
 */
struct GTACharacter : public GameObject
{
private:
	GTAVehicle* currentVehicle;
	size_t currentSeat;

	void createActor(const glm::vec3& size = glm::vec3(0.35f, 0.35f, 1.3f));
	void destroyActor();

	// Incredibly hacky "move in this direction".
	bool _hasTargetPosition;
	glm::vec3 _targetPosition;

public:

	enum Action {
		None,
		Idle,
		Walk,
		Run,
		Crouch,
		Jump,
		Falling,
		Landing,
		VehicleGetIn,
		VehicleDrive,
		VehicleSit,
		KnockedDown,
		GettingUp
	};

	std::shared_ptr<CharacterData> ped;

	btKinematicCharacterController* physCharacter;
	btPairCachingGhostObject* physObject;
	btCapsuleShapeZ* physShape;

	GTAAIController* controller;

	AnimationGroup animations;

	/**
	 * @brief GTACharacter Constructs a Character
	 * @param pos
	 * @param rot
	 * @param model
	 * @param ped PEDS_t struct to use.
	 */
	GTACharacter(GameWorld* engine, const glm::vec3& pos, const glm::quat& rot, Model* model, std::shared_ptr<CharacterData> data);

	~GTACharacter();

	Type type() { return Character; }

	Action currentActivity;

	void enterAction(Action act);

	void tick(float dt);

	/**
	 * @brief updateCharacter updates internall bullet Character.
	 */
	void updateCharacter(float dt);

	virtual void setPosition(const glm::vec3& pos);

	virtual glm::vec3 getPosition() const;
	
	bool enterVehicle(GTAVehicle* vehicle, size_t seat);

	GTAVehicle *getCurrentVehicle() const;
	size_t getCurrentSeat() const;
	void setCurrentVehicle(GTAVehicle *value, size_t seat);
	
    virtual bool takeDamage(const DamageInfo& damage);
	
	void jump();
	
	/**
	 * Resets the Actor to the nearest AI Graph node
	 * (taking into account the current vehicle)
	 */
	void resetToAINode();

	void setTargetPosition( const glm::vec3& target );
};

#endif
