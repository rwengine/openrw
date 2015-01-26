#pragma once
#ifndef _GAMEWORLD_HPP_
#define _GAMEWORLD_HPP_

#include <engine/GameData.hpp>
#include <engine/GameState.hpp>
#include <render/GameRenderer.hpp>

#include <ai/AIGraphNode.hpp>
#include <ai/AIGraph.hpp>

class WorkContext;

class GameObject;
class CharacterObject;
class InstanceObject;
class VehicleObject;

struct WeaponScan;

class ScriptMachine;

#include <glm/glm.hpp>

#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>

#include <vector>
#include <set>
#include <queue>
#include <random>

/**
 * Information about "Goal" locations so they can be rendered
 * (this doesn't really belong here).
 */
struct AreaIndicatorInfo
{
	enum AreaIndicatorType
	{
		Cylinder
	};
	
	AreaIndicatorType type;
	glm::vec3 position;
	glm::vec3 radius;
};


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
	 * Creates an instance
	 */
	InstanceObject *createInstance(const uint16_t id, const glm::vec3& pos, const glm::quat& rot = glm::quat());
	uint16_t findModelDefinition(const std::string model);

	/**
	 * @brief Creates an InstanceObject for use in the current Cutscene.
	 */
	CutsceneObject *createCutsceneObject(const uint16_t id, const glm::vec3& pos, const glm::quat& rot = glm::quat());
	
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
	 * @brief Put an object on the deletion queue.
	 */
	void destroyObjectQueued(GameObject* object);

	/**
	 * @brief Destroys all objects on the destruction queue.
	 */
	void destroyQueuedObjects();

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
	 * Object Definitions
	 */
	std::map<ObjectID, ObjectInformationPtr> objectTypes;

	template<class T> std::shared_ptr<T> findObjectType(ObjectID id)
	{
		auto f = objectTypes.find(id);
		/// @TODO don't instanciate an object here just to read .type
		T tmp;
		if( f != objectTypes.end() && f->second->class_type == tmp.class_type )
		{
			return std::static_pointer_cast<T>(f->second);
		}
		return nullptr;
	}

	/**
	* Paths associated with each object definition.
	*/
	std::map<uint16_t, std::vector<std::shared_ptr<PathData>>> objectNodes;

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

	/**
	 * @brief Loads and starts the named cutscene.
	 * @param name
	 */
	void loadCutscene(const std::string& name);
	void startCutscene();
	void clearCutscene();

	sf::SoundStream* cutsceneAudio;
	bool cutsceneAudioLoaded;
	sf::SoundBuffer missionAudio;
	sf::Sound missionSound;

	/**
	 * @brief loads a model into a special character slot.
	 */
	void loadSpecialCharacter(const unsigned short index, const std::string& name);
	void loadSpecialModel(const unsigned short index, const std::string& name);
	
	void disableAIPaths(AIGraphNode::NodeType type, const glm::vec3& min, const glm::vec3& max);
	void enableAIPaths(AIGraphNode::NodeType type, const glm::vec3& min, const glm::vec3& max);
	
	void drawAreaIndicator(AreaIndicatorInfo::AreaIndicatorType type, glm::vec3 position, glm::vec3 radius);
	
	const std::vector<AreaIndicatorInfo>& getAreaIndicators() const { return areaIndicators; }
	
	void clearTickData();

private:

	/**
	 * @brief Used by objects to delete themselves during updates.
	 */
	std::queue<GameObject*> deletionQueue;

	std::vector<AreaIndicatorInfo> areaIndicators;
};

#endif
