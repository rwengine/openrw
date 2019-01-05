#include "engine/GameWorld.hpp"

#ifdef _MSC_VER
#pragma warning(disable : 4305 5033)
#endif
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <btBulletDynamicsCommon.h>
#ifdef _MSC_VER
#pragma warning(default : 4305 5033)
#endif

#include <glm/gtx/norm.hpp>

#include <data/Clump.hpp>

#include "core/Profiler.hpp"
#include "core/Logger.hpp"

#include "engine/GameData.hpp"
#include "engine/GameState.hpp"

#include "ai/DefaultAIController.hpp"
#include "ai/PlayerController.hpp"
#include "ai/TrafficDirector.hpp"

#include "dynamics/HitTest.hpp"

#include "data/CutsceneData.hpp"
#include "data/InstanceData.hpp"

#include "items/Weapon.hpp"

#include "loaders/LoaderCutsceneDAT.hpp"
#include "loaders/LoaderIFP.hpp"
#include "loaders/LoaderIPL.hpp"

#include "objects/CharacterObject.hpp"
#include "objects/CutsceneObject.hpp"
#include "objects/InstanceObject.hpp"
#include "objects/PickupObject.hpp"
#include "objects/VehicleObject.hpp"

#include "platform/FileHandle.hpp"

#include "render/ViewCamera.hpp"

#ifdef RW_WINDOWS
#include <rw_mingw.hpp>
#endif

// Behaviour Tuning
constexpr float kMaxTrafficSpawnRadius = 100.f;
constexpr float kMaxTrafficCleanupRadius = kMaxTrafficSpawnRadius * 1.25f;

namespace {
template <typename T>
bool shouldEffectBeRemoved(const T& effect, float gameTime) {
    if (effect->getType() != Particle) {
        return false;
    }
    auto particle = static_cast<ParticleFX*>(effect.get());
    return particle->lifetime >= 0.f &&
           gameTime >= particle->starttime + particle->lifetime;
}
}  // namespace

class WorldCollisionDispatcher : public btCollisionDispatcher {
public:
    WorldCollisionDispatcher(btCollisionConfiguration* collisionConfiguration)
        : btCollisionDispatcher(collisionConfiguration) {
    }

    bool needsResponse(const btCollisionObject* obA,
                       const btCollisionObject* obB) override {
        return btCollisionDispatcher::needsResponse(obA, obB);
    }
};

GameWorld::GameWorld(Logger* log, GameData* dat)
    : logger(log), data(dat), sound(this) {
    data->engine = this;

    collisionConfig = std::make_unique<btDefaultCollisionConfiguration>();
    collisionDispatcher =
        std::make_unique<WorldCollisionDispatcher>(collisionConfig.get());
    broadphase = std::make_unique<btDbvtBroadphase>();
    solver = std::make_unique<btSequentialImpulseConstraintSolver>();
    dynamicsWorld = std::make_unique<btDiscreteDynamicsWorld>(
        collisionDispatcher.get(), broadphase.get(), solver.get(),
        collisionConfig.get());

    dynamicsWorld->setGravity(btVector3(0.f, 0.f, -9.81f));
    _overlappingPairCallback = std::make_unique<btGhostPairCallback>();
    broadphase->getOverlappingPairCache()->setInternalGhostPairCallback(
        _overlappingPairCallback.get());
    gContactProcessedCallback = ContactProcessedCallback;
    dynamicsWorld->setInternalTickCallback(PhysicsTickCallback, this);
    dynamicsWorld->setForceUpdateAllAabbs(false);
}

GameWorld::~GameWorld() {
    // Bullet requires to remove each object before all physic world
    pedestrianPool.clear();
    instancePool.clear();
    vehiclePool.clear();
    pickupPool.clear();
    cutscenePool.clear();
    projectilePool.clear();
}

bool GameWorld::placeItems(const std::string& name) {
    LoaderIPL ipll;

    if (ipll.load(name)) {
        // Find the object.
        for (const auto& inst : ipll.m_instances) {
            if (!createInstance(inst->id, inst->pos, inst->rot)) {
                logger->error("World", "No object data for instance " +
                                           std::to_string(inst->id) + " in " +
                                           name);
            }
        }

        return true;
    } else {
        logger->error("Data", "Failed to load IPL " + name);
        return false;
    }

    return false;
}

InstanceObject* GameWorld::createInstance(const uint16_t id,
                                          const glm::vec3& pos,
                                          const glm::quat& rot) {
    auto oi = data->findModelInfo<SimpleModelInfo>(id);
    if (oi) {
        // Request loading of the model if it isn't loaded already.
        /// @todo implment streaming properly
        if (!oi->isLoaded()) {
            data->loadModel(oi->id());
        }

        // Check for dynamic data.
        const auto dyIt = data->dynamicObjectData.find(oi->name);

        DynamicObjectData* dydata = nullptr;

        if (dyIt != data->dynamicObjectData.end()) {
            dydata = &dyIt->second;
        }

        if (oi->name.empty()) {
            logger->warning(
                "World", "Instance with missing model: " + std::to_string(id));
        }

        auto instance =
            std::make_unique<InstanceObject>(this, pos, rot, glm::vec3(1.f), oi, dydata);

        auto ptr = instance.get();

        instancePool.insert(std::move(instance));
        allObjects.push_back(ptr);

        modelInstances.emplace(oi->name, ptr);

        return ptr;
    }

    return nullptr;
}

void GameWorld::createTraffic(const ViewCamera& viewCamera) {
    TrafficDirector director(&aigraph, this);

    director.populateNearby(viewCamera, kMaxTrafficSpawnRadius, 5);
}

void GameWorld::cleanupTraffic(const ViewCamera& focus) {
    for (auto& p : pedestrianPool.objects) {
        if (p.second->getLifetime() != GameObject::TrafficLifetime) {
            continue;
        }

        if (glm::distance(focus.position, p.second->getPosition()) >=
            kMaxTrafficCleanupRadius) {
            if (!focus.frustum.intersects(p.second->getPosition(), 1.f)) {
                destroyObjectQueued(p.second.get());
            }
        }
    }
    for (auto& p : vehiclePool.objects) {
        if (p.second->getLifetime() != GameObject::TrafficLifetime) {
            continue;
        }

        if (glm::distance(focus.position, p.second->getPosition()) >=
            kMaxTrafficCleanupRadius) {
            if (!focus.frustum.intersects(p.second->getPosition(), 1.f)) {
                destroyObjectQueued(p.second.get());
            }
        }
    }

    destroyQueuedObjects();
}

CutsceneObject* GameWorld::createCutsceneObject(const uint16_t id,
                                                const glm::vec3& pos,
                                                const glm::quat& rot) {
    auto modelinfo = data->modelinfo[id].get();

    if (!modelinfo) {
        return nullptr;
    }

    auto clumpmodel = static_cast<ClumpModelInfo*>(modelinfo);

    if (!clumpmodel->isLoaded()) {
        data->loadModel(id);
    }
    auto model = clumpmodel->getModel();

    if (id == 0) {
        auto playerobj = pedestrianPool.find(state->playerObject);
        if (playerobj) {
            model = playerobj->getModel();
        }
    }

    auto instance = std::make_unique<CutsceneObject>(this, pos, rot, model, modelinfo);
    auto ptr = instance.get();

    cutscenePool.insert(std::move(instance));
    allObjects.push_back(ptr);

    return ptr;
}

VehicleObject* GameWorld::createVehicle(const uint16_t id, const glm::vec3& pos,
                                        const glm::quat& rot,
                                        GameObjectID gid) {
    auto vti = data->findModelInfo<VehicleModelInfo>(id);
    if (!vti) {
        return nullptr;
    }
    logger->info("World", "Creating Vehicle ID " + std::to_string(id) + " (" +
                              vti->vehiclename_ + ")");

    if (!vti->isLoaded()) {
        data->loadModel(id);
    }

    glm::u8vec3 prim(255), sec(128);
    auto palit = data->vehiclePalettes.find(
        vti->name);  // modelname is conveniently lowercase (usually)
    if (palit != data->vehiclePalettes.end() && !palit->second.empty()) {
        size_t set = getRandomNumber(0u, palit->second.size() - 1);
        prim = data->vehicleColours[palit->second[set].first];
        sec = data->vehicleColours[palit->second[set].second];
    } else {
        logger->warning("World", "No colour palette for vehicle " + vti->name);
    }

    auto addSeats = [](std::vector<SeatInfo>& seats, glm::vec3&& offset) {
        // Left seat
        offset.x = -offset.x;
        seats.push_back({offset});

        // Right seat
        offset.x = -offset.x;
        seats.push_back({offset});
    };

    auto model = vti->getModel();
    auto info = data->vehicleInfos.find(vti->handling_);
    if (model && info != data->vehicleInfos.end() &&
        info->second.wheels.empty() && info->second.seats.empty()) {
        auto root = model->getFrame();
        for (const auto& frame : root->getChildren()) {
            const std::string& name = frame->getName();

            if (name.size() > 5 && name.substr(0, 5) == "wheel") {
                const auto& frameTrans = frame->getWorldTransform();
                info->second.wheels.push_back({glm::vec3(frameTrans[3])});
            }

            if (name == "chassis_dummy") {
                // These are nested within chassis_dummy
                auto frontseat = frame->findDescendant("ped_frontseat");
                auto backseat = frame->findDescendant("ped_backseat");

                if (frontseat) {
                    addSeats(info->second.seats.front,
                             frontseat->getDefaultTranslation());
                }
                if (backseat) {
                    // @todo how does this work for the barracks, ambulance
                    // or coach?
                    addSeats(info->second.seats.back,
                             backseat->getDefaultTranslation());
                }
            } else if (name == "ped_frontseat") {
                // The speeder boat does not have a chassis_dummy but has the
                // frontseat directly in the root frame.

                addSeats(info->second.seats.front,
                         frame->getDefaultTranslation());
            }
        }
    }

    auto vehicle =
        std::make_unique<VehicleObject>(this, pos, rot, vti, &info->second, prim, sec);
    auto ptr = vehicle.get();
    vehicle->setGameObjectID(gid);

    vehiclePool.insert(std::move(vehicle));
    allObjects.push_back(ptr);

    return ptr;
}

CharacterObject* GameWorld::createPedestrian(const uint16_t id,
                                             const glm::vec3& pos,
                                             const glm::quat& rot,
                                             GameObjectID gid) {
    auto pt = data->findModelInfo<PedModelInfo>(id);
    if (!pt) {
        return nullptr;
    }

    auto isSpecial = pt->name.compare(0, 7, "special") == 0;
    if (!pt->isLoaded() || isSpecial) {
        data->loadModel(id);
    }

    auto controller = new DefaultAIController();
    auto ped = std::make_unique<CharacterObject>(this, pos, rot, pt, controller);
    auto ptr = ped.get();
    ped->setGameObjectID(gid);
    pedestrianPool.insert(std::move(ped));
    allObjects.push_back(ptr);
    return ptr;
}

CharacterObject* GameWorld::createPlayer(const glm::vec3& pos,
                                         const glm::quat& rot,
                                         GameObjectID gid) {
    // Player object ID is hardcoded to 0.
    auto pt = data->findModelInfo<PedModelInfo>(0);
    if (!pt) {
        return nullptr;
    }

    // Model name is also hardcoded.
    std::string modelname = "player";
    std::string texturename = "player";

    data->loadTXD(texturename + ".txd");
    if (!pt->isLoaded()) {
        auto model = data->loadClump(modelname + ".dff");
        pt->setModel(model);
    }

    auto controller = new PlayerController();
    auto ped = std::make_unique<CharacterObject>(this, pos, rot, pt, controller);
    auto ptr = ped.get();
    ped->setGameObjectID(gid);
    ped->setLifetime(GameObject::PlayerLifetime);
    players.push_back(controller);
    pedestrianPool.insert(std::move(ped));
    allObjects.push_back(ptr);
    return ptr;
}

PickupObject* GameWorld::createPickup(const glm::vec3& pos, int id, int type) {
    auto modelInfo = data->modelinfo[id].get();

    RW_CHECK(modelInfo != nullptr, "Pickup Object Data is not found");
    if (modelInfo == nullptr) {
        return nullptr;
    }

    if (!modelInfo->isLoaded()) {
        data->loadModel(id);
    }

    std::unique_ptr<PickupObject> pickup;
    auto pickuptype = static_cast<PickupObject::PickupType>(type);

    auto it = std::find_if(
        data->weaponData.begin(), data->weaponData.end(),
        [=](const auto& x) { return x.modelID == id; });

    // If nothing, create a generic pickup instead of an item pickup
    if (it != data->weaponData.end()) {
        pickup = std::make_unique<ItemPickup>(this, pos, modelInfo, pickuptype, *it);
    } else if (modelInfo->name == "info" || modelInfo->name == "briefcase" ||
               modelInfo->name == "floatpackge1") {
        pickup = std::make_unique<DummyPickup>(this, pos, modelInfo, pickuptype);
    } else if (modelInfo->name == "killfrenzy") {
        pickup = std::make_unique<RampagePickup>(this, pos, modelInfo, pickuptype);
    } else if (modelInfo->name == "health") {
        pickup = std::make_unique<HealthPickup>(this, pos, modelInfo, pickuptype);
    } else if (modelInfo->name == "bodyarmour") {
        pickup = std::make_unique<ArmourPickup>(this, pos, modelInfo, pickuptype);
    } else if (modelInfo->name == "package1") {
        pickup = std::make_unique<CollectablePickup>(this, pos, modelInfo, pickuptype);
    } else if (modelInfo->name == "adrenaline") {
        pickup = std::make_unique<AdrenalinePickup>(this, pos, modelInfo, pickuptype);
    } else if (modelInfo->name == "Money") {
        pickup = std::make_unique<MoneyPickup>(this, pos, modelInfo, pickuptype, 0);
     } else if (modelInfo->name == "donkeymag") {
        pickup = std::make_unique<BigNVeinyPickup>(this, pos, modelInfo, pickuptype);
        pickup->setBehaviourFlags(PickupObject::BehaviourFlags::PickupInVehicle);
    } else {
        RW_UNIMPLEMENTED("Non-weapon pickups");
        pickup = std::make_unique<PickupObject>(this, pos, modelInfo, pickuptype);
    }

    auto ptr = pickup.get();

    pickupPool.insert(std::move(pickup));
    allObjects.push_back(ptr);

    return ptr;
}

Garage* GameWorld::createGarage(const glm::vec3 coord0, const glm::vec3 coord1,
                                Garage::Type type) {
    const size_t id = garages.size();
    garages.emplace_back(std::make_unique<Garage>(this, id, coord0, coord1, type));
    return garages.back().get();
}

Payphone* GameWorld::createPayphone(const glm::vec2 coord) {
    const size_t id = payphones.size();
    payphones.emplace_back(std::make_unique<Payphone>(this, id, coord));
    return payphones.back().get();
}

void GameWorld::ObjectPool::insert(std::unique_ptr<GameObject> object) {
    if (object->getGameObjectID() == 0) {
        // Find the lowest free GameObjectID.
        GameObjectID availID = 1;
        for (auto& [id, objectPtr] : objects) {
            if (id == availID) availID++;
        }

        object->setGameObjectID(availID);
    }
    objects[object->getGameObjectID()] = std::move(object);
}

GameObject* GameWorld::ObjectPool::find(GameObjectID id) const {
    auto it = objects.find(id);
    return (it == objects.end()) ? nullptr : it->second.get();
}

void GameWorld::ObjectPool::remove(GameObject* object) {
    if (object) {
        auto it = objects.find(object->getGameObjectID());
        if (it != objects.end()) {
            it = objects.erase(it);
        }
    }
}

void GameWorld::ObjectPool::clear() {
    objects.clear();
}

GameWorld::ObjectPool& GameWorld::getTypeObjectPool(GameObject* object) {
    switch (object->type()) {
        case GameObject::Character:
            return pedestrianPool;
        case GameObject::Vehicle:
            return vehiclePool;
        case GameObject::Cutscene:
            return cutscenePool;
        case GameObject::Instance:
            return instancePool;
        case GameObject::Pickup:
            return pickupPool;
        case GameObject::Projectile:
            return projectilePool;
        default:
            // error!
            return pedestrianPool;
    }
}

GameObject* GameWorld::getBlipTarget(const BlipData& blip) const {
    switch (blip.type) {
        case BlipData::Vehicle:
            return vehiclePool.find(blip.target);
        case BlipData::Character:
            return pedestrianPool.find(blip.target);
        case BlipData::Pickup:
            return pickupPool.find(blip.target);
        case BlipData::Instance:
            return instancePool.find(blip.target);
        default:
            return nullptr;
    }
}

void GameWorld::destroyObject(GameObject* object) {
    auto& pool = getTypeObjectPool(object);
    pool.remove(object);

    // Remove from mission objects
    if (state) {
        auto& mO = state->missionObjects;
        mO.erase(std::remove(mO.begin(), mO.end(), object), mO.end());
    }

    auto it = std::find(allObjects.begin(), allObjects.end(), object);
    RW_CHECK(it != allObjects.end(), "destroying object not in allObjects");
    if (it != allObjects.end()) {
        allObjects.erase(it);
    }
}

void GameWorld::destroyObjectQueued(GameObject* object) {
    RW_CHECK(object != nullptr, "destroying a null object?");
    if (object) deletionQueue.insert(object);
}

void GameWorld::destroyQueuedObjects() {
    while (!deletionQueue.empty()) {
        destroyObject(*deletionQueue.begin());
        deletionQueue.erase(deletionQueue.begin());
    }
}

LightFX& GameWorld::createLightEffect() {
    auto effect = std::make_unique<LightFX>();
    auto& ref = *effect;
    effects.push_back(std::move(effect));
    return ref;
}

ParticleFX& GameWorld::createParticleEffect() {
    auto effect = std::make_unique<ParticleFX>();
    auto& ref = *effect;
    effects.push_back(std::move(effect));
    return ref;
}

TrailFX& GameWorld::createTrailEffect() {
    auto effect = std::make_unique<TrailFX>();
    auto& ref = *effect;
    effects.push_back(std::move(effect));
    return ref;
}

void GameWorld::destroyEffect(VisualFX& effect) {
    auto found =
        std::find_if(effects.begin(), effects.end(),
                     [&effect](auto& ef) { return ef.get() == &effect; });

    if (found != effects.end()) {
        effects.erase(found);
    }
}

void GameWorld::doWeaponScan(const WeaponScan& scan) {
    if (scan.type == ScanType::Radius) {
        HitTest test {*dynamicsWorld};
        const auto& result = test.sphereTest(scan.center, scan.radius);

        for(const auto& target : result) {
            if (!scan.doesDamage(target.object)) {
                continue;
            }

            target.object->takeDamage(
                {
                    GameObject::DamageInfo::DamageType::Melee,
                    {}, scan.center, scan.damage
                });
        }

    } else if (scan.type == ScanType::HitScan) {
        btVector3 from(scan.center.x, scan.center.y, scan.center.z),
            to(scan.end.x, scan.end.y, scan.end.z);
        btCollisionWorld::ClosestRayResultCallback cb(from, to);
        cb.m_collisionFilterGroup = btBroadphaseProxy::AllFilter;
        dynamicsWorld->rayTest(from, to, cb);
        if (!cb.hasHit()) {
            return;
        }

        auto go = static_cast<GameObject *>(
            cb.m_collisionObject->getUserPointer());
        go->takeDamage(
            {
                GameObject::DamageInfo::DamageType::Bullet,
                {cb.m_hitPointWorld.x(), cb.m_hitPointWorld.y(),
                 cb.m_hitPointWorld.z()},
                scan.center, scan.damage
            });
    }
}

int GameWorld::getHour() {
    return state->basic.gameHour;
}

int GameWorld::getMinute() {
    return state->basic.gameMinute;
}

void GameWorld::offsetGameTime(int minutes) {
    int gameMinute = state->basic.gameMinute;
    int gameHour = state->basic.gameHour;

    gameMinute += minutes;
    bool backwards = gameMinute < 0;

    gameHour += gameMinute / 60 - (backwards ? 1 : 0);

    // Black mathgic? No. If the value is negative we want to wrap it around
    // to the other side, e.g. minute -7 should be 53.
    // The equivalent would be doing "while (gameMinute < 0) { gameMinute += 60
    // }"
    gameMinute = (gameMinute % 60 + 60) % 60;
    gameHour = (gameHour % 24 + 24) % 24;

    state->basic.gameMinute = gameMinute;
    state->basic.gameHour = gameHour;
}

glm::vec3 GameWorld::getGroundAtPosition(const glm::vec3& pos) const {
    btVector3 rayFrom(pos.x, pos.y, 100.f);
    btVector3 rayTo(pos.x, pos.y, -100.f);

    btDynamicsWorld::ClosestRayResultCallback rr(rayFrom, rayTo);

    dynamicsWorld->rayTest(rayFrom, rayTo, rr);

    if (rr.hasHit()) {
        auto& ws = rr.m_hitPointWorld;
        return {ws.x(), ws.y(), ws.z()};
    }

    return pos;
}

float GameWorld::getGameTime() const {
    return state->gameTime;
}

namespace {
void handleVehicleResponse(GameObject* object, btManifoldPoint& mp, bool isA) {
    bool isVehicle = object->type() == GameObject::Vehicle;
    if (!isVehicle) return;
    if (mp.getAppliedImpulse() <= 100.f) return;

    btVector3 src, dmg;
    if (isA) {
        src = mp.getPositionWorldOnB();
        dmg = mp.getPositionWorldOnA();
    } else {
        src = mp.getPositionWorldOnA();
        dmg = mp.getPositionWorldOnB();
    }

    object->takeDamage({
                           GameObject::DamageInfo::DamageType::Physics,
                           {dmg.x(), dmg.y(), dmg.z()},
                           {src.x(), src.y(), src.z()},
                           0.f,
                           mp.getAppliedImpulse()
                       });
}

void handleInstanceResponse(InstanceObject* instance, const btManifoldPoint& mp,
                            bool isA) {
    if (!instance->dynamics) {
        return;
    }

    auto dmg = isA ? mp.m_positionWorldOnA : mp.m_positionWorldOnB;
    auto impulse = mp.getAppliedImpulse();

    if (impulse <= 0.0f) {
        return;
    }

    ///@ todo Correctness: object damage calculation
    constexpr auto kMinimumDamageImpulse = 500.f;
    const auto hp = std::max(0.f, impulse - kMinimumDamageImpulse);
    instance->takeDamage({
                             GameObject::DamageInfo::DamageType::Physics,
                             {dmg.x(), dmg.y(), dmg.z()},
                             {dmg.x(), dmg.y(), dmg.z()},
                             hp,
                             impulse
                         });
}
}  // namespace

bool GameWorld::ContactProcessedCallback(btManifoldPoint& mp, void* body0,
                                         void* body1) {
    RW_PROFILE_SCOPEC(__func__, MP_GOLDENROD1);
    auto obA = static_cast<btCollisionObject*>(body0);
    auto obB = static_cast<btCollisionObject*>(body1);

    if (!(obA->getUserPointer() && obB->getUserPointer())) {
        return false;
    }

    GameObject* a = static_cast<GameObject*>(obA->getUserPointer());
    GameObject* b = static_cast<GameObject*>(obB->getUserPointer());

    bool aIsInstance = a && a->type() == GameObject::Instance;
    bool bIsInstance = b && b->type() == GameObject::Instance;

    bool exactly_one_is_instance = aIsInstance != bIsInstance;

    if (exactly_one_is_instance) {
        InstanceObject* instance = nullptr;

        if (aIsInstance) {
            instance = static_cast<InstanceObject*>(a);
        } else {
            instance = static_cast<InstanceObject*>(b);
        }

        handleInstanceResponse(instance, mp, aIsInstance);
    }

    // Handle vehicles
    if (a) handleVehicleResponse(a, mp, true);
    if (b) handleVehicleResponse(b, mp, false);

    return true;
}

void GameWorld::PhysicsTickCallback(btDynamicsWorld* physWorld,
                                    btScalar timeStep) {
    RW_PROFILE_SCOPEC(__func__, MP_CYAN);
    GameWorld* world = static_cast<GameWorld*>(physWorld->getWorldUserInfo());

    RW_PROFILE_COUNTER_SET("physicsTick/vehiclePool", world->vehiclePool.objects.size());
    for (auto& p : world->vehiclePool.objects) {
        RW_PROFILE_SCOPEC("VehicleObject", MP_THISTLE1);
        auto object = static_cast<VehicleObject*>(p.second.get());
        object->tickPhysics(timeStep);
    }

    RW_PROFILE_COUNTER_SET("physicsTick/pedestrianPool", world->pedestrianPool.objects.size());
    for (auto& p : world->pedestrianPool.objects) {
        RW_PROFILE_SCOPEC("CharacterObject", MP_THISTLE1);
        auto object = static_cast<CharacterObject*>(p.second.get());
        object->tickPhysics(timeStep);
    }

    RW_PROFILE_COUNTER_SET("physicsTick/instancePool", world->instancePool.objects.size());
    for (auto& p : world->instancePool.objects) {
        auto object = static_cast<InstanceObject*>(p.second.get());
        object->tickPhysics(timeStep);
    }
}

void GameWorld::loadCutscene(const std::string& name) {
    auto datfile = data->index.openFile(name + ".dat");

    state->currentCutscene = CutsceneData();

    if (datfile.data) {
        LoaderCutsceneDAT loaderdat;
        loaderdat.load(state->currentCutscene->tracks, datfile);
    }

    data->loadIFP(name + ".ifp", true);
    cutsceneAudioLoaded = data->loadAudioStream(name + ".mp3");

    if (!cutsceneAudioLoaded) {
        cutsceneAudioLoaded = data->loadAudioStream(name + ".wav");
    }

    if (!cutsceneAudioLoaded) {
        logger->warning("Data", "Failed to load cutscene audio: " + name);
    }

    state->currentCutscene->meta.name = name;
    logger->info("World", "Loaded cutscene: " + name);
}

void GameWorld::startCutscene() {
    state->cutsceneStartTime = getGameTime();
    state->skipCutscene = false;

    if (cutsceneAudio.length() > 0) {
        sound.pauseAllSounds();
        sound.playMusic(cutsceneAudio);
    }
}

void GameWorld::clearCutscene() {
    eraseCutsceneObjects();
    eraseCutsceneSound();
    eraseCutsceneAnimations();

    state->currentCutscene = std::nullopt;
    state->isCinematic = false;
    state->cutsceneStartTime = -1.f;
}

void GameWorld::eraseCutsceneObjects() {
    for (auto& p : cutscenePool.objects) {
        destroyObjectQueued(p.second.get());
    }
}

void GameWorld::eraseCutsceneSound() {
    if (cutsceneAudio.length() > 0) {
        sound.stopMusic(cutsceneAudio);
        sound.eraseSound(cutsceneAudio);
        cutsceneAudio = "";
        sound.resumeAllSounds();
    }
}

void GameWorld::eraseCutsceneAnimations() {
    data->animationsCutscene.clear();
}

bool GameWorld::isCutsceneDone() {
    if (state->currentCutscene) {
        float time = getGameTime() - state->cutsceneStartTime;
        if (state->skipCutscene) {
            return true;
        }
        return time > state->currentCutscene->tracks.duration;
    }
    return true;
}

void GameWorld::loadSpecialCharacter(const unsigned short index,
                                     const std::string& name) {
    constexpr uint16_t kFirstSpecialActor = 26;
    logger->info("Data", "Loading special actor " + name + " to " +
                             std::to_string(index));
    auto modelid = kFirstSpecialActor + index - 1;
    auto model = data->findModelInfo<PedModelInfo>(modelid);
    if (model && model->isLoaded()) {
        model->unload();
    }
    std::string lowerName(name);
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(),
                   ::tolower);
    state->specialCharacters[index] = lowerName;
}

void GameWorld::loadSpecialModel(const unsigned short index,
                                 const std::string& name) {
    logger->info("Data", "Loading cutscene object " + name + " to " +
                             std::to_string(index));
    // Tell the HIER model to discard the currently loaded model
    auto model = data->findModelInfo<ClumpModelInfo>(index);
    if (model && model->isLoaded()) {
        model->unload();
    }
    std::string lowerName(name);
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(),
                   ::tolower);
    state->specialModels[index] = lowerName;
}

void GameWorld::disableAIPaths(AIGraphNode::NodeType type, const glm::vec3& min,
                               const glm::vec3& max) {
    for (auto& n : aigraph.nodes) {
        if (n->type == type) {
            if (n->position.x >= min.x && n->position.y >= min.y &&
                n->position.z >= min.z && n->position.x <= max.x &&
                n->position.y <= max.y && n->position.z <= max.z) {
                n->disabled = true;
            }
        }
    }
}

void GameWorld::enableAIPaths(AIGraphNode::NodeType type, const glm::vec3& min,
                              const glm::vec3& max) {
    for (auto& n : aigraph.nodes) {
        if (n->type == type) {
            if (n->position.x >= min.x && n->position.y >= min.y &&
                n->position.z >= min.z && n->position.x <= max.x &&
                n->position.y <= max.y && n->position.z <= max.z) {
                n->disabled = false;
            }
        }
    }
}

void GameWorld::drawAreaIndicator(AreaIndicatorInfo::AreaIndicatorType type,
                                  glm::vec3 position, glm::vec3 radius) {
    areaIndicators.push_back({type, position, radius});
}

void GameWorld::clearTickData() {
    areaIndicators.clear();
}

void GameWorld::setPaused(bool pause) {
    paused = pause;
    bool resumingCutscene = !pause && !isCutsceneDone();
    if (resumingCutscene) {
        sound.playMusic(cutsceneAudio);
    } else {
        sound.pause(pause);
    }
}

bool GameWorld::isPaused() const {
    return paused;
}

void GameWorld::updateEffects() {
    effects.erase(std::remove_if(effects.begin(), effects.end(),
                                 [gameTime = getGameTime()](auto& effect) {
                                     return shouldEffectBeRemoved(effect,
                                                                  gameTime);
                                 }),
                  effects.end());
}

VehicleObject* GameWorld::tryToSpawnVehicle(VehicleGenerator& gen) {
    constexpr float kMinClearRadius = 10.f;

    if (gen.remainingSpawns <= 0) {
        return nullptr;
    }

    /// @todo take into account maxDelay as well
    if (gen.lastSpawnTime + gen.minDelay >
        static_cast<int>(state->basic.timeMS)) {
        return nullptr;
    }

    /// @todo verify this logic
    auto position = gen.position;
    if (position.z < -90.f) {
        position = getGroundAtPosition(position);
    }

    // Ensure there's no existing vehicles near our spawn point
    for (auto& v : vehiclePool.objects) {
        if (glm::distance2(position, v.second->getPosition()) <
            kMinClearRadius * kMinClearRadius) {
            return nullptr;
        }
    }

    int id = gen.vehicleID;
    if (id == -1) {
        // Random ID found by dice roll
        id = 134;
        /// @todo use zone information to decide vehicle id
    }

    auto model = data->findModelInfo<VehicleModelInfo>(id);
    RW_ASSERT(model);
    if (model) {
        auto info = data->vehicleInfos.find(model->handling_);
        if (info != data->vehicleInfos.end()) {
            const auto& handling = info->second.handling;
            position.z +=
                (handling.dimensions.z / 2.f) - handling.centerOfMass.z;
        }
    }

    auto vehicle = createVehicle(id, position);
    vehicle->setHeading(gen.heading);
    vehicle->setLifetime(GameObject::TrafficLifetime);

/// @todo apply vehicle colours
/// @todo apply locked & alarm thresholds

// According to http://www.gtamodding.com/wiki/014C the spawn limit
// doesn't work.
#if 0
	if (gen.remainingSpawns < 101) {
		gen.remainingSpawns --;
	}
#endif

    gen.lastSpawnTime = state->basic.timeMS;

    return vehicle;
}

bool GameWorld::isRaining() const {
    return WeatherCondition(state->basic.nextWeather) ==
           WeatherCondition::Rainy;
}

void GameWorld::clearObjectsWithinArea(const glm::vec3 center,
                                       const float radius,
                                       const bool clearParticles) {
    bool skipFlag = false;

    // Vehicles
    for (auto& obj : vehiclePool.objects) {
        skipFlag = false;

        // Skip if it's the player or owned by player or owned by mission
        if (obj.second->getLifetime() == GameObject::PlayerLifetime ||
            obj.second->getLifetime() == GameObject::MissionLifetime) {
            continue;
        }

        // Check if we have any important objects in a vehicle, if we do - don't
        // erase it
        for (auto& seat :
             static_cast<VehicleObject*>(obj.second.get())->seatOccupants) {
            auto character = static_cast<CharacterObject*>(seat.second);

            if (character->getLifetime() == GameObject::PlayerLifetime ||
                character->getLifetime() == GameObject::MissionLifetime) {
                skipFlag = true;
            }
        }

        if (skipFlag) {
            continue;
        }

        if (glm::distance(center, obj.second->getPosition()) < radius) {
            destroyObjectQueued(obj.second.get());
        }
    }

    // Peds
    for (auto& obj : pedestrianPool.objects) {
        // Skip if it's the player or owned by player or owned by mission
        if (obj.second->getLifetime() == GameObject::PlayerLifetime ||
            obj.second->getLifetime() == GameObject::MissionLifetime) {
            continue;
        }

        if (glm::distance(center, obj.second->getPosition()) < radius) {
            destroyObjectQueued(obj.second.get());
        }
    }

    /// @todo Do we also have to clear all projectiles + particles *in this
    /// area*, even if the bool is false?

    if (clearParticles) {
        RW_UNUSED(clearParticles);
        RW_UNIMPLEMENTED(
            "should clear all particles and projectiles (not limited to "
            "area!)");
    }

    // @todo Remove all temp objects, extinguish all fires, remove all
    // explosions, remove all projectiles
}

PlayerController* GameWorld::getPlayer() {
    auto object = pedestrianPool.find(state->playerObject);
    if (object) {
        auto controller = static_cast<CharacterObject*>(object)->controller;
        return static_cast<PlayerController*>(controller);
    }
    return nullptr;
}
