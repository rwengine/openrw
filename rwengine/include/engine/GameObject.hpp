#pragma once
#ifndef _GAMEOBJECT_HPP_
#define _GAMEOBJECT_HPP_

#include <engine/RWTypes.hpp>
#include <loaders/LoaderIDE.hpp>
#include <loaders/LoaderIPL.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>

class CharacterController;
class ModelFrame;
class Animator;

class GameWorld;

/**
 * Stores data used by call types of object instances.
 */
struct GameObject
{
    glm::vec3 position;
    glm::quat rotation;

	ModelHandle* model; /// Cached pointer to Object's Model.
    
    GameWorld* engine;

	Animator* animator; /// Object's animator.

	/**
	 * Health value
	 */
	float mHealth;

	bool _inWater;

	// Used to determine in water status
	float _lastHeight;

	GameObject(GameWorld* engine, const glm::vec3& pos, const glm::quat& rot, ModelHandle* model)
		: position(pos), rotation(rot), model(model), engine(engine), animator(nullptr), mHealth(0.f),
		  _inWater(false), _lastHeight(std::numeric_limits<float>::max())
	{}
		
	virtual ~GameObject() {}

    enum Type
    {
        Instance,
        Character,
		Vehicle,
		Unknown
    };

	virtual Type type() { return Unknown; }
	
	virtual void setPosition(const glm::vec3& pos);
	
	virtual glm::vec3 getPosition() const;

	virtual glm::quat getRotation() const;
	
	struct DamageInfo
	{
		enum DamageType
		{
			Explosion,
			Burning,
			Bullet,
			Physics
		};
		
		/**
		 * World position of damage
		 */
		glm::vec3 damageLocation;
		
		/**
		 * World position of the source (used for direction)
		 */
		glm::vec3 damageSource;
		
		/**
		 * Magnitude of destruction
		 */
		float hitpoints;
		
		/**
		 * Type of the damage
		 */
		DamageType type;

		/**
		 * Physics impulse.
		 */
		float impulse;
	};
	
	virtual bool takeDamage(const DamageInfo& damage) { return false; }

	virtual bool isAnimationFixed() const { return true; }

	virtual bool isInWater() const { return _inWater; }
};

#endif // __GAMEOBJECTS_HPP__
