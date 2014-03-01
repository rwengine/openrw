#pragma once
#ifndef _GAMEOBJECT_HPP_
#define _GAMEOBJECT_HPP_

#include <engine/GTATypes.hpp>
#include <loaders/LoaderIDE.hpp>
#include <loaders/LoaderIPL.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>

class GTAAIController;
class Model;
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

    Model* model; /// Cached pointer to Object's Model.
    
    GameWorld* engine;

	Animator* animator; /// Object's animator.
	
	/**
	 * Health value
	 */
	float mHealth; 

	GameObject(GameWorld* engine, const glm::vec3& pos, const glm::quat& rot, Model* model)
		: position(pos), rotation(rot), model(model), engine(engine), animator(nullptr), mHealth(0.f) {}
		
	virtual ~GameObject() {};

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
	};
	
	virtual bool takeDamage(const DamageInfo& damage) { return false; }

	virtual bool isFrameVisible(ModelFrame* frame) const { return true; }
};

#endif // __GAMEOBJECTS_HPP__
