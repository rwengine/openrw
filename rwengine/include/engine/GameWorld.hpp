#pragma once
#ifndef _GAMEWORLD_HPP_
#define _GAMEWORLD_HPP_

#include <engine/GameData.hpp>
#include <engine/GameState.hpp>
#include <render/GameRenderer.hpp>

#include <loaders/LoaderIPL.hpp>
#include <ai/AIGraphNode.hpp>
#include <ai/AIGraph.hpp>

class WorkContext;

class GameObject;
class CharacterObject;
class InstanceObject;
class VehicleObject;

class WeaponScan;

class ScriptMachine;

#include <glm/glm.hpp>

#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>

#include <vector>
#include <set>
#include <queue>
#include <random>

/**
 * @brief Handles all data relating to object instances and other "worldly" state.
 */
class GameWorld
{
public:

	GameWorld(const std::string& gamepath);

	~GameWorld();
	
	/**
	 * Loads the game data
	 */
	bool load();
	
	struct LogEntry
	{
		enum Type {
			Info,
			Error,
			Warning
		};
		
		Type type;
		float time;
		std::string message;
	};
	
	std::deque<LogEntry> log;
	
	/**
	 * Displays an informative message
	 */
	void logInfo(const std::string& info);
	
	/**
	 * Displays an alarming error
	 */
	void logError(const std::string& error);
	
	/**
	 * Displays a comforting warning
	 */
    void logWarning(const std::string& warning);
	
	/**
	 * Loads an IDE into the game
	 */
	bool defineItems(const std::string& name);

	void runScript(const std::string& name);
	
	/**
	 * Loads an IPL into the game.
	 * @param name The name of the IPL as it appears in the games' gta.dat
	 */
	bool placeItems(const std::string& name);
	
	/**
	 * Loads the Zones from a zon/IPL file
	 */
	bool loadZone(const std::string& path);
	
	/**
	 * Creates an instance
	 */
	InstanceObject *createInstance(const uint16_t id, const glm::vec3& pos, const glm::quat& rot = glm::quat());
	
	/**
	 * Creates a vehicle
	 */
	VehicleObject *createVehicle(const uint16_t id, const glm::vec3& pos, const glm::quat& rot = glm::quat());

    /**
     * Creates a pedestrian.
     */
	CharacterObject* createPedestrian(const uint16_t id, const glm::vec3& pos, const glm::quat& rot = glm::quat());
	
	/**
	 * Destroys an existing Object
	 */
	void destroyObject(GameObject* object);

	/**
	 * Performs a weapon scan against things in the world
	 */
	void doWeaponScan(const WeaponScan &scan );
	
	/**
	 * Returns the current hour
	 */
	int getHour();
	
	/**
	 * Returns the current minute
	 */
	int getMinute();

	glm::vec3 getGroundAtPosition(const glm::vec3& pos) const;
	
	/** 
	 * Game Clock
	 */
	float gameTime;
	
	/**
	 * Game data
	 */
	GameData gameData;
	
	/**
	 * Renderer
	 */
	GameRenderer renderer;

	/**
	 * Gameplay state
	 */
	GameState state;
	
	/**
	 * Map Zones
	 */
	std::map<std::string, LoaderIPL::Zone> zones;
	
	/**
	 * Object Definitions
	 */
	std::map<uint16_t, std::shared_ptr<ObjectData>> objectTypes;

	/**
	* Paths associated with each object definition.
	*/
	std::map<uint16_t, std::vector<std::shared_ptr<PathData>>> objectNodes;

	/**
	 * Vehicle type definitions
	 * @todo move this non-instance data to GameData
	 */
	std::map<uint16_t, std::shared_ptr<VehicleData>> vehicleTypes;

	/**
	* Ped definitions
	* @todo move this non-instance data to GameData
	*/
	std::map<uint16_t, std::shared_ptr<CharacterData>> pedestrianTypes;

	/**
	 * @brief objects All active GameObjects in the world.
	 * @todo add some mechanism to allow objects to be "locked" preventing deletion.
	 * @todo add deletion queue to allow objects to self delete.
	 */
	std::set<GameObject*> objects;

	/**
	 * Map of Model Names to Instances
	 */
	std::map<std::string, InstanceObject*> modelInstances;

	/**
	 * AI Graph
	 */
	AIGraph aigraph;

	/**
	 * Randomness Engine
	 */
	std::default_random_engine randomEngine;
	
	/**
	 * Bullet 
	 */
	btDefaultCollisionConfiguration* collisionConfig;
	btCollisionDispatcher* collisionDispatcher;
	btBroadphaseInterface* broadphase;
	btSequentialImpulseConstraintSolver* solver;
	btDiscreteDynamicsWorld* dynamicsWorld;

	/**
	 * @brief physicsNearCallback
	 * Used to implement uprooting and other physics oddities.
	 */
	static bool ContactProcessedCallback(btManifoldPoint& mp, void* body0, void* body1);

	/**
	 * @brief PhysicsTickCallback updates object each physics tick.
	 * @param physWorld
	 * @param timeStep
	 */
	static void PhysicsTickCallback(btDynamicsWorld* physWorld, btScalar timeStep);

	/**
	 * Work related
	 */
	WorkContext* _work;

	ScriptMachine* script;
};

#endif
