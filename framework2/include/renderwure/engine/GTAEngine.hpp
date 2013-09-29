#pragma once
#ifndef _GTAENGINE_HPP_
#define _GTAENGINE_HPP_

#include <renderwure/engine/GTAData.hpp>
#include <renderwure/render/GTARenderer.hpp>
#include <renderwure/loaders/LoaderIPL.hpp>
#include <renderwure/ai/GTAAINode.hpp>
#include <renderwure/ai/AIGraph.hpp>

class GTAObject;
class GTACharacter;
class GTAInstance;
class GTAVehicle;

#include <glm/glm.hpp>

#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>

#include <vector>
#include <queue>
#include <random>

/**
 * @class GTAEngine
 *  Provides a simple interface to the framework's internals
 */
class GTAEngine
{
public:

	GTAEngine(const std::string& gamepath);
	
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
	GTAInstance *createInstance(const uint16_t id, const glm::vec3& pos, const glm::quat& rot = glm::quat());
	
	/**
	 * Creates a vehicle
	 */
    GTAVehicle *createVehicle(const uint16_t id, const glm::vec3& pos, const glm::quat& rot = glm::quat());

    /**
     * Creates a pedestrian.
     */
    GTACharacter* createPedestrian(const uint16_t id, const glm::vec3& pos, const glm::quat& rot = glm::quat());
	
	/**
	 * Roughly the middle of everything
	 */
	glm::vec3 itemCentroid; 
	size_t itemCount;
	
	/** 
	 * Game Clock
	 */
	float gameTime;
	
	/**
	 * Game data
	 */
	GTAData gameData;
	
	/**
	 * Renderer
	 */
	GTARenderer renderer;
	
	/**
	 * Map Zones
	 */
	std::vector<LoaderIPL::Zone> zones;
	
	/**
	 * Object Definitions
	 */
	std::map<uint16_t, std::shared_ptr<LoaderIDE::OBJS_t>> objectTypes;
	
    /**
     * Paths associated with each object definition.
     */
    std::map<uint16_t, std::vector<LoaderIDE::PATH_t>> objectNodes;
	
	/**
	 * Vehicle definitions
	 */
	std::map<uint16_t, std::shared_ptr<LoaderIDE::CARS_t>> vehicleTypes;

    /**
     * Ped definitions
     */
    std::map<uint16_t, std::shared_ptr<LoaderIDE::PEDS_t>> pedestrianTypes;
	
	/**
	 * Game Objects!
	 */
	std::vector<std::shared_ptr<GTAInstance>> objectInstances;
	
	/**
	 * Map of Model Names to Instances
	 */
	std::map<std::string, std::shared_ptr<GTAInstance>> modelInstances;
	
	/**
	 * Game Vehicles!
	 */
	std::vector<GTAVehicle*> vehicleInstances;

    /**
     * Pedestrians and PCs.
     */
    std::vector<GTACharacter*> pedestrians;

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
	
};

#endif
