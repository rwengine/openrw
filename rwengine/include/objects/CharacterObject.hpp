#pragma once
#ifndef _CHARACTEROBJECT_HPP_
#define _CHARACTEROBJECT_HPP_
#include <objects/GameObject.hpp>
#include <bullet/BulletDynamics/Character/btKinematicCharacterController.h>
#include <bullet/btBulletCollisionCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <glm/glm.hpp>

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

	AnimationGroup()
	 : idle(nullptr), walk(nullptr), walk_start(nullptr), run(nullptr),
	   jump_start(nullptr), jump_glide(nullptr), jump_land(nullptr),
	   car_sit(nullptr), car_sit_low(nullptr), car_open_lhs(nullptr),
	   car_getin_lhs(nullptr), car_getout_lhs(nullptr), car_open_rhs(nullptr),
	   car_getin_rhs(nullptr), car_getout_rhs(nullptr)
	{}
};

/**
 * @brief The CharacterObject struct
 * Implements Character object behaviours.
 */
class CharacterObject : public GameObject
{
private:
	VehicleObject* currentVehicle;
	size_t currentSeat;

	void createActor(const glm::vec2& size = glm::vec2(0.7f, 0.8f));
	void destroyActor();

	// Incredibly hacky "move in this direction".
	bool _hasTargetPosition;
	glm::vec3 _targetPosition;

	std::map<int, InventoryItem*> _inventory;
	int _activeInventoryItem;

	bool jumped = false;
	float jumpSpeed;
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
	
	bool enterVehicle(VehicleObject* vehicle, size_t seat);

	VehicleObject *getCurrentVehicle() const;
	size_t getCurrentSeat() const;
	void setCurrentVehicle(VehicleObject *value, size_t seat);
	
    virtual bool takeDamage(const DamageInfo& damage);
	
	void jump();
	void setJumpSpeed(float speed);
	float getJumpSpeed() const;
	
	/**
	 * Resets the Actor to the nearest AI Graph node
	 * (taking into account the current vehicle)
	 */
	void resetToAINode();

	void setTargetPosition( const glm::vec3& target );
	void clearTargetPosition();

	void playAnimation(Animation* animation, bool repeat);

	void addToInventory( InventoryItem* item );
	void setActiveItem( int slot );
	InventoryItem* getActiveItem();
	void destroyItem( int slot );

	void cycleInventory( bool up );

	const std::map<int, InventoryItem*>& getInventory() const { return _inventory; }
};

#endif
