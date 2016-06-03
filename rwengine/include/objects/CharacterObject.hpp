#pragma once
#ifndef _CHARACTEROBJECT_HPP_
#define _CHARACTEROBJECT_HPP_
#include <objects/GameObject.hpp>
#include <bullet/BulletDynamics/Character/btKinematicCharacterController.h>
#include <bullet/btBulletCollisionCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <glm/glm.hpp>

constexpr int maxInventorySlots = 13;

// Animation slots used for character animation blending
constexpr unsigned int AnimIndexMovement = 0;
constexpr unsigned int AnimIndexAction = 1;

struct CharacterWeaponSlot
{
	// Assuming these match the entries in weapon.dat
	uint32_t weaponId;
	uint32_t bulletsClip;
	uint32_t bulletsTotal;
};

struct CharacterState
{
	glm::vec3 position;
	float rotation;
	float health = 100.f;
	float armour = 0.f;
	std::array<CharacterWeaponSlot, maxInventorySlots> weapons;
	uint16_t currentWeapon = 0;
	uint32_t lastFireTimeMS = 0;
	bool primaryActive = false;
	bool secondaryActive = false;
	uint32_t primaryStartTime = 0;
	uint32_t primaryEndTime = 0;
};

class VehicleObject;
class GameWorld;
class InventoryItem;

struct AnimationGroup
{
	Animation* idle;
	Animation* walk;
	Animation* walk_start;
	Animation* run;

	Animation* walk_right;
	Animation* walk_right_start;
	Animation* walk_left;
	Animation* walk_left_start;

	Animation* walk_back;
	Animation* walk_back_start;

	Animation* jump_start;
	Animation* jump_glide;
	Animation* jump_land;

	Animation* car_sit;
	Animation* car_sit_low;

	Animation* car_open_lhs;
	Animation* car_getin_lhs;
	Animation* car_getout_lhs;

	Animation* car_open_rhs;
	Animation* car_getin_rhs;
	Animation* car_getout_rhs;

	Animation* kd_front;
	Animation* ko_shot_front;

	AnimationGroup()
	 : idle(nullptr), walk(nullptr), walk_start(nullptr), run(nullptr),
	   jump_start(nullptr), jump_glide(nullptr), jump_land(nullptr),
	   car_sit(nullptr), car_sit_low(nullptr), car_open_lhs(nullptr),
	   car_getin_lhs(nullptr), car_getout_lhs(nullptr), car_open_rhs(nullptr),
	   car_getin_rhs(nullptr)
	 , car_getout_rhs(nullptr)
	 , kd_front(nullptr)
	 , ko_shot_front(nullptr)
	{}
};

/**
 * @brief The CharacterObject struct
 * Implements Character object behaviours.
 */
class CharacterObject : public GameObject
{
private:
	CharacterState currentState;

	VehicleObject* currentVehicle;
	size_t currentSeat;

	void createActor(const glm::vec2& size = glm::vec2(0.45f, 1.2f));
	void destroyActor();

	glm::vec3 movement;
	glm::vec2 m_look;

	bool running;
	bool jumped;
	float jumpSpeed;

	bool motionBlockedByActivity;

	glm::vec3 updateMovementAnimation(float dt);
public:

	static const float DefaultJumpSpeed;

	std::shared_ptr<CharacterData> ped;

	btKinematicCharacterController* physCharacter;
	btPairCachingGhostObject* physObject;
	btCapsuleShapeZ* physShape;

	CharacterController* controller;

	AnimationGroup animations;

	/**
	 * @param pos
	 * @param rot
	 * @param model
	 * @param ped PEDS_t struct to use.
	 */
	CharacterObject(GameWorld* engine, const glm::vec3& pos, const glm::quat& rot, const ModelRef& model, std::shared_ptr< CharacterData > data);

	~CharacterObject();

	Type type() { return Character; }

	void tick(float dt);

	const CharacterState& getCurrentState() const { return currentState; }
	CharacterState& getCurrentState(){ return currentState; }

	/**
	 * @brief Loads the model and texture for a character skin.
	 */
	void changeCharacterModel(const std::string& name);

	/**
	 * @brief updateCharacter updates internall bullet Character.
	 */
	void updateCharacter(float dt);

	virtual void setPosition(const glm::vec3& pos);

	virtual glm::vec3 getPosition() const;

	virtual glm::quat getRotation() const;

	bool isAlive() const;
	bool takeDamage(const DamageInfo& damage) override;

	bool enterVehicle(VehicleObject* vehicle, size_t seat);

	/**
	 * @brief isStopped
	 * @return True if the character isn't moving
	 */
	bool isStopped() const;

	VehicleObject *getCurrentVehicle() const;
	size_t getCurrentSeat() const;
	void setCurrentVehicle(VehicleObject *value, size_t seat);
	
	void jump();
	void setJumpSpeed(float speed);
	float getJumpSpeed() const;
	bool isOnGround() const;
	bool canTurn() const;

	void setRunning(bool run) { running = run; }
	bool isRunning() const { return running; }
	
	/**
	 * Resets the Actor to the nearest AI Graph node
	 * (taking into account the current vehicle)
	 */
	void resetToAINode();

	void setMovement(const glm::vec3& _m) { movement = _m; }
	const glm::vec3& getMovement() const { return movement; }
	void setLook(const glm::vec2& look) { m_look = look; }
	const glm::vec2& getLook() const { return m_look; }

	/**
	 * @brief playActivityAnimation Plays an animation for an activity.
	 * @param animation The animation to play
	 * @param repeat
	 * @param blocking Wether movement is still alowed
	 *
	 * This allows controller activities to play their own animations and
	 * controll blending with movement.
	 */
	void playActivityAnimation(Animation* animation, bool repeat, bool blocking);
	/**
	 * @brief activityFinished removes activity animation
	 */
	void activityFinished();

	void addToInventory( InventoryItem* item );
	void setActiveItem( int slot );
	InventoryItem* getActiveItem();
	void removeFromInventory( int slot );

	/**
	 * Uses the character's active item.
	 * @param primary use the primary action.
	 */
	void useItem(bool active, bool primary = true);

	void cycleInventory( bool up );
};

#endif
