#pragma once
#ifndef _GAMEOBJECT_HPP_
#define _GAMEOBJECT_HPP_

#include <rw/types.hpp>
#include <objects/ObjectTypes.hpp>
#include <loaders/LoaderIDE.hpp>
#include <loaders/LoaderIPL.hpp>
#include <data/Model.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

class Skeleton;
class CharacterController;
class ModelFrame;
class Animator;
class GameWorld;

/**
 * @brief Base data and interface for all world "objects" like vehicles, peds.
 *
 * Contains handle to the world, and other useful properties like water level
 * tracking used to make tunnels work.
 */
class GameObject
{
	glm::vec3 _lastPosition;
	glm::quat _lastRotation;
	GameObjectID objectID;
public:
    glm::vec3 position;
    glm::quat rotation;

	/// Reference to Model data
	ModelRef model;
	
    GameWorld* engine;

	Animator* animator; /// Object's animator.
	Skeleton* skeleton;

	bool inWater;

	/**
	 * @brief stores the height of water at the last tick
	 */
	float _lastHeight;
	
	/**
	 * Should object be rendered?
	 */
	bool visible;

	GameObject(GameWorld* engine, const glm::vec3& pos, const glm::quat& rot, ModelRef model)
		: _lastPosition(pos)
		, _lastRotation(rot)
		, objectID(0)
		, position(pos)
		, rotation(rot)
		, model(model)
		, engine(engine)
		, animator(nullptr)
		, skeleton(nullptr)
		, inWater(false)
		, _lastHeight(std::numeric_limits<float>::max())
		, visible(true)
		, lifetime(GameObject::UnknownLifetime)
	{}
		
	virtual ~GameObject();

	GameObjectID getGameObjectID() const { return objectID; }
	/**
	 * Do not call this, use GameWorld::insertObject
	 */
	void setGameObjectID(GameObjectID id) { objectID = id; }

	int getScriptObjectID() const { return getGameObjectID(); }

	/**
	 * @brief Enumeration of possible object types.
	 */
    enum Type
    {
		Instance,
		Character,
		Vehicle,
		Pickup,
		Projectile,
		Cutscene,
		Unknown
    };

	/**
	 * @brief determines what type of object this is.
	 * @return one of Type
	 */
	virtual Type type() { return Unknown; }
	
	virtual void setPosition(const glm::vec3& pos);

	const glm::vec3& getPosition() const { return position; }
	const glm::vec3& getLastPosition() const { return _lastPosition; }

	const glm::quat& getRotation() const { return rotation; }
	virtual void setRotation(const glm::quat &orientation);

	float getHeading() const;
	/**
	 * @brief setHeading Rotates the object to face heading, in degrees.
	 */
	void setHeading(float heading);

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
	
	virtual bool takeDamage(const DamageInfo& /*damage*/) { return false; }

	virtual bool isAnimationFixed() const { return true; }

	virtual bool isInWater() const { return inWater; }

	virtual void tick(float dt) = 0;

	/**
	 * @brief Function used to modify the last transform
	 * @param newPos
	 */
	void _updateLastTransform()
	{
		_lastPosition = getPosition();
		_lastRotation = getRotation();
	}

	glm::mat4 getTimeAdjustedTransform(float alpha) const {
		glm::mat4 t;
		t = glm::translate(t, glm::mix(_lastPosition, getPosition(), alpha));
		t = t * glm::mat4_cast(glm::slerp(_lastRotation, getRotation(), alpha));
		return t;
	}
	
	enum ObjectLifetime
	{
		/// lifetime has not been set
		UnknownLifetime,
		/// Generic background pedestrians
		TrafficLifetime,
		/// Part of a mission
		MissionLifetime,
		/// Is owned by the player (or is the player)
		PlayerLifetime
	};
	
	void setLifetime(ObjectLifetime ol) { lifetime = ol; }
	ObjectLifetime getLifetime() const { return lifetime; }

	void updateTransform(const glm::vec3& pos, const glm::quat& rot)
	{
		_lastPosition = position;
		_lastRotation = rotation;
		position = pos;
		rotation = rot;
	}

private:
	ObjectLifetime lifetime;
};

#endif // __GAMEOBJECTS_HPP__
