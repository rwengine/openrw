#pragma once
#ifndef _GAMEWORLD_HPP_
#define _GAMEWORLD_HPP_

#include <engine/GameData.hpp>
#include <render/GameRenderer.hpp>
#include <loaders/LoaderIPL.hpp>
#include <ai/AIGraphNode.hpp>
#include <ai/AIGraph.hpp>

class WorkContext;

class GameObject;
class CharacterObject;
class InstanceObject;
class VehicleObject;

#include <glm/glm.hpp>

#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>

#include <vector>
#include <queue>
#include <random>

/**
 * @class GameWorld
 *  Represents a single instance of the game world, and all of the data required.
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
	 * Returns the current hour
	 */
    int getHour();
	
	/**
	 * Returns the current minute
	 */
	int getMinute();
	
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
	 * Map Zones
	 */
	std::vector<LoaderIPL::Zone> zones;
	
	/**
	 * Object Definitions
	 */
	std::map<uint16_t, std::shared_ptr<ObjectData>> objectTypes;

	/**
		* Paths associated with each object definition.
		*/
	std::map<uint16_t, std::vector<std::shared_ptr<PathData>>> objectNodes;

	/**
	 * Vehicle definitions
	 */
	std::map<uint16_t, std::shared_ptr<VehicleData>> vehicleTypes;

	/**
		* Ped definitions
		*/
	std::map<uint16_t, std::shared_ptr<CharacterData>> pedestrianTypes;
	
	/**
	 * Game Objects!
	 */
	std::vector<std::shared_ptr<InstanceObject>> objectInstances;
	
	/**
	 * Map of Model Names to Instances
	 */
	std::map<std::string, std::shared_ptr<InstanceObject>> modelInstances;
	
	/**
	 * Game Vehicles!
	 */
	std::vector<VehicleObject*> vehicleInstances;

    /**
     * Pedestrians and PCs.
     */
	std::vector<CharacterObject*> pedestrians;
	
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
	 * @brief handleCollisionResponses performs physics response checking
	 * for collisions between vehicles, objects etc.
	 */
	void handleCollisionResponses();

	/**
	 * Work related
	 */
	WorkContext* _work;
};

#endif
