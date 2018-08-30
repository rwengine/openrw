#ifndef _RWENGINE_GAMEWORLD_HPP_
#define _RWENGINE_GAMEWORLD_HPP_

#include <cstdint>
#include <map>
#include <memory>
#include <random>
#include <set>
#include <string>
#include <vector>

#ifdef _MSC_VER
#pragma warning(disable : 4305)
#endif
#include <LinearMath/btScalar.h>
#ifdef _MSC_VER
#pragma warning(default : 4305)
#endif

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <ai/AIGraph.hpp>
#include <ai/AIGraphNode.hpp>
#include <audio/SoundManager.hpp>

#include <engine/Garage.hpp>
#include <engine/Payphone.hpp>
#include <objects/ObjectTypes.hpp>

#include <render/VisualFX.hpp>

#include <data/Chase.hpp>

class btCollisionDispatcher;
class btDefaultCollisionConfiguration;
class btDiscreteDynamicsWorld;
class btDynamicsWorld;
class btManifoldPoint;
class btOverlappingPairCallback;
class btSequentialImpulseConstraintSolver;
struct btDbvtBroadphase;

class GameState;
class Garage;
class Payphone;

class PlayerController;
class Logger;

class GameData;
class CutsceneObject;

class GameObject;
class CharacterObject;
class InstanceObject;
class VehicleObject;
class PickupObject;

class ViewCamera;

struct BlipData;
struct WeaponScan;
struct VehicleGenerator;

/**
 * Information about "Goal" locations so they can be rendered
 * (this doesn't really belong here).
 */
struct AreaIndicatorInfo {
    enum AreaIndicatorType { Cylinder };

    AreaIndicatorType type;
    glm::vec3 position{};
    glm::vec3 radius{};
};

/**
 * @brief Handles all data relating to object instances and other "worldly"
 * state.
 */
class GameWorld {
public:
    GameWorld(Logger* log, GameData* dat);

    ~GameWorld();

    Logger* logger;

    /**
     * Loads an IPL into the game.
     * @param name The name of the IPL as it appears in the games' gta.dat
     */
    bool placeItems(const std::string& name);

    /**
     * @brief createTraffic spawn transitory peds and vehicles
     * @param viewCamera The camera to create traffic near
     *
     * The position and frustum of the passed in camera is used to determine
     * the radius where traffic can be spawned, and the frustum is used to avoid
     * spawning traffic in view of the player.
     */
    void createTraffic(const ViewCamera& viewCamera);

    /**
     * @brief cleanupTraffic Cleans up traffic too far away from the given
     * camera
     * @param viewCamera
     */
    void cleanupTraffic(const ViewCamera& viewCamera);

    /**
     * Creates an instance
     */
    InstanceObject* createInstance(const uint16_t id, const glm::vec3& pos,
                                   const glm::quat& rot = glm::quat{
                                       1.0f, 0.0f, 0.0f, 0.0f});

    /**
     * @brief Creates an InstanceObject for use in the current Cutscene.
     */
    CutsceneObject* createCutsceneObject(const uint16_t id,
                                         const glm::vec3& pos,
                                         const glm::quat& rot = glm::quat{
                                             1.0f, 0.0f, 0.0f, 0.0f});

    /**
     * Creates a vehicle
     */
    VehicleObject* createVehicle(const uint16_t id, const glm::vec3& pos,
                                 const glm::quat& rot = glm::quat{1.0f, 0.0f,
                                                                  0.0f, 0.0f},
                                 GameObjectID gid = 0);

    /**
     * Creates a pedestrian.
     */
    CharacterObject* createPedestrian(
        const uint16_t id, const glm::vec3& pos,
        const glm::quat& rot = glm::quat{1.0f, 0.0f, 0.0f, 0.0f},
        GameObjectID gid = 0);

    /**
     * Creates a player
     */
    CharacterObject* createPlayer(const glm::vec3& pos,
                                  const glm::quat& rot = glm::quat{1.0f, 0.0f,
                                                                   0.0f, 0.0f},
                                  GameObjectID gid = 0);

    /**
     * Creates a pickup
     */
    PickupObject* createPickup(const glm::vec3& pos, int id, int type);

    /**
     * Creates a garage
     */
    Garage* createGarage(const glm::vec3 coord0, const glm::vec3 coord1,
                         Garage::Type type);

    /**
     * Creates a payphone
     */
    Payphone* createPayphone(const glm::vec2 coord);

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
    void doWeaponScan(const WeaponScan& scan);

    /**
     * Allocates a new Light Effect
     */
    LightFX& createLightEffect();

    /**
     * Allocates a new Particle Effect
     */
    ParticleFX& createParticleEffect();

    /**
     * Allocates a new Trail Effect
     */
    TrailFX& createTrailEffect();

    /**
     * Immediately destoys the given effect
     */
    void destroyEffect(VisualFX& effect);

    /**
     * Returns the current hour
     */
    int getHour();

    /**
     * Returns the current minute
     */
    int getMinute();

    /**
     * Modifies the game time and handles the circular nature of clock numbers
     * Supports negative numbers
     */
    void offsetGameTime(int minutes);

    //! Check if the weather conditions are rainy
    bool isRaining() const;

    glm::vec3 getGroundAtPosition(const glm::vec3& pos) const;

    float getGameTime() const;

    /**
     * Game data
     */
    GameData* data;

    /**
     * Gameplay state
     */
    GameState* state;

    /**
     * State of playing sounds
     */
    SoundManager sound;

    /**
     * Chase state
     */
    ChaseCoordinator chase;

    /**
     * Each object type is allocated from a pool. This object helps manage
     * the individual pools.
     */
    struct ObjectPool {
        std::map<GameObjectID, GameObject*> objects;

        /**
         * Allocates the game object a GameObjectID and inserts it into
         * the pool
         */
        void insert(GameObject* object);

        /**
         * Removes a game object from this pool
         */
        void remove(GameObject* object);

        /**
         * Finds a game object if it exists in this pool
         */
        GameObject* find(GameObjectID id) const;
    };

    /**
     * Stores all game objects
     */
    std::vector<GameObject*> allObjects;

    ObjectPool pedestrianPool;
    ObjectPool instancePool;
    ObjectPool vehiclePool;
    ObjectPool pickupPool;
    ObjectPool cutscenePool;
    ObjectPool projectilePool;

    ObjectPool& getTypeObjectPool(GameObject* object);

    std::vector<PlayerController*> players;

    std::vector<std::unique_ptr<Garage>> garages;

    std::vector<std::unique_ptr<Payphone>> payphones;

    /**
     * @brief getBlipTarget
     * @param blip
     * @return The targetted object of the given blip
     */
    GameObject* getBlipTarget(const BlipData& blip) const;

    /**
     * Map of Model Names to Instances
     */
    std::map<std::string, InstanceObject*> modelInstances;

    /**
     * AI Graph
     */
    AIGraph aigraph;

    /**
     * Visual Effects
     * @todo Consider using lighter handing mechanism
     */
    std::vector<std::unique_ptr<VisualFX>> effects;

    /**
     * Randomness Engine
     */
    std::default_random_engine randomEngine{std::random_device{}()};

    /**
     * Bullet
     */
    std::unique_ptr<btDefaultCollisionConfiguration> collisionConfig;
    std::unique_ptr<btCollisionDispatcher> collisionDispatcher;
    std::unique_ptr<btDbvtBroadphase> broadphase;
    std::unique_ptr<btSequentialImpulseConstraintSolver> solver;
    std::unique_ptr<btDiscreteDynamicsWorld> dynamicsWorld;

    /**
     * @brief physicsNearCallback
     * Used to implement uprooting and other physics oddities.
     */
    static bool ContactProcessedCallback(btManifoldPoint& mp, void* body0,
                                         void* body1);

    /**
     * @brief PhysicsTickCallback updates object each physics tick.
     * @param physWorld
     * @param timeStep
     */
    static void PhysicsTickCallback(btDynamicsWorld* physWorld,
                                    btScalar timeStep);

    /**
     * @brief Loads and starts the named cutscene.
     * @param name
     */
    void loadCutscene(const std::string& name);
    void startCutscene();
    void clearCutscene();
    bool isCutsceneDone();

    std::string cutsceneAudio;
    bool cutsceneAudioLoaded;
    std::string missionAudio;

    /**
     * @brief loads a model into a special character slot.
     */
    void loadSpecialCharacter(const unsigned short index,
                              const std::string& name);
    void loadSpecialModel(const unsigned short index, const std::string& name);

    void disableAIPaths(AIGraphNode::NodeType type, const glm::vec3& min,
                        const glm::vec3& max);
    void enableAIPaths(AIGraphNode::NodeType type, const glm::vec3& min,
                       const glm::vec3& max);

    void drawAreaIndicator(AreaIndicatorInfo::AreaIndicatorType type,
                           glm::vec3 position, glm::vec3 radius);

    const std::vector<AreaIndicatorInfo>& getAreaIndicators() const {
        return areaIndicators;
    }

    void clearTickData();

    void setPaused(bool pause);
    bool isPaused() const;

    /**
     * Clean up old VisualFX
     */
    void updateEffects();

    /**
     * Attempt to spawn a vehicle at a vehicle generator
     */
    VehicleObject* tryToSpawnVehicle(VehicleGenerator& gen);

    void clearObjectsWithinArea(const glm::vec3 center, const float radius,
                                const bool clearParticles);

    PlayerController* getPlayer();

private:
    /**
     * @brief Used by objects to delete themselves during updates.
     */
    std::set<GameObject*> deletionQueue;

    std::vector<AreaIndicatorInfo> areaIndicators;

    /**
     * Flag for pausing the simulation
     */
    bool paused = false;

    /**
     * Private data
     */
    std::unique_ptr<btOverlappingPairCallback> _overlappingPairCallback;
};

#endif
