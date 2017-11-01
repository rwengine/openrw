#include "engine/GameWorld.hpp"

#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <btBulletDynamicsCommon.h>

#include <glm/gtx/norm.hpp>

#include <data/Clump.hpp>

#include "core/Logger.hpp"

#include "engine/GameData.hpp"
#include "engine/GameState.hpp"

#include "ai/DefaultAIController.hpp"
#include "ai/PlayerController.hpp"
#include "ai/TrafficDirector.hpp"

#include "data/CutsceneData.hpp"
#include "data/InstanceData.hpp"
#include "data/WeaponData.hpp"

#include "loaders/LoaderCutsceneDAT.hpp"
#include "loaders/LoaderIPL.hpp"

#include "objects/CharacterObject.hpp"
#include "objects/CutsceneObject.hpp"
#include "objects/InstanceObject.hpp"
#include "objects/ItemPickup.hpp"
#include "objects/VehicleObject.hpp"

#include "render/ViewCamera.hpp"

#ifdef RW_WINDOWS
#include <rw_mingw.hpp>
#endif


// Behaviour Tuning
constexpr float kMaxTrafficSpawnRadius = 100.f;
constexpr float kMaxTrafficCleanupRadius = kMaxTrafficSpawnRadius * 1.25f;

class WorldCollisionDispatcher : public btCollisionDispatcher {
public:
    WorldCollisionDispatcher(btCollisionConfiguration* collisionConfiguration)
        : btCollisionDispatcher(collisionConfiguration) {
    }

    bool needsResponse(const btCollisionObject* obA,
                       const btCollisionObject* obB) {
        if (!(obA->getUserPointer() && obB->getUserPointer())) {
            return btCollisionDispatcher::needsResponse(obA, obB);
        }

        GameObject* a = static_cast<GameObject*>(obA->getUserPointer());
        GameObject* b = static_cast<GameObject*>(obB->getUserPointer());

        bool aIsInstance = a && a->type() == GameObject::Instance;
        bool bIsInstance = b && b->type() == GameObject::Instance;

        bool exactly_one_gameobject_is_instance = aIsInstance != bIsInstance;

        if (exactly_one_gameobject_is_instance) {
            // Figure out which is the dynamic instance.
            InstanceObject* dynInst = nullptr;
            const btRigidBody *instBody = nullptr, *otherBody = nullptr;

            if (aIsInstance) {
                dynInst = static_cast<InstanceObject*>(a);
                instBody = static_cast<const btRigidBody*>(obA);
                otherBody = static_cast<const btRigidBody*>(obB);
            } else {
                dynInst = static_cast<InstanceObject*>(b);
                instBody = static_cast<const btRigidBody*>(obB);
                otherBody = static_cast<const btRigidBody*>(obA);
            }

            if (dynInst->dynamics == nullptr || !instBody->isStaticObject()) {
                return btCollisionDispatcher::needsResponse(obA, obB);
            }

            // Attempt to determine relative velocity.
            auto dV = (otherBody->getLinearVelocity());
            auto impulse = dV.length();

            // Ignore collision if the object is about to be uprooted.
            if (dynInst->dynamics->uprootForce <=
                impulse / (otherBody->getInvMass())) {
                return false;
            }
        }
        return btCollisionDispatcher::needsResponse(obA, obB);
    }
};

GameWorld::GameWorld(Logger* log, GameData* dat)
    : logger(log), data(dat), randomEngine(rand()), paused(false) {
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
}

GameWorld::~GameWorld() {
    for (auto& p : allObjects) {
        delete p;
    }
}

bool GameWorld::placeItems(const std::string& name) {
    std::string path = name;

    LoaderIPL ipll;

    if (ipll.load(path)) {
        // Find the object.
        for (size_t i = 0; i < ipll.m_instances.size(); ++i) {
            std::shared_ptr<InstanceData> inst = ipll.m_instances[i];
            if (!createInstance(inst->id, inst->pos, inst->rot)) {
                logger->error("World", "No object data for instance " +
                                           std::to_string(inst->id) + " in " +
                                           path);
            }
        }

        return true;
    } else {
        logger->error("Data", "Failed to load IPL " + path);
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
        auto dyit = data->dynamicObjectData.find(oi->name);
        std::shared_ptr<DynamicObjectData> dydata;
        if (dyit != data->dynamicObjectData.end()) {
            dydata = dyit->second;
        }

        if (oi->name.empty()) {
            logger->warning(
                "World", "Instance with missing model: " + std::to_string(id));
        }

        auto instance =
            new InstanceObject(this, pos, rot, glm::vec3(1.f), oi, dydata);

        instancePool.insert(instance);
        allObjects.push_back(instance);

        modelInstances.insert({oi->name, instance});

        return instance;
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
                destroyObjectQueued(p.second);
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
                destroyObjectQueued(p.second);
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

    auto instance = new CutsceneObject(this, pos, rot, model, modelinfo);

    cutscenePool.insert(instance);
    allObjects.push_back(instance);

    return instance;
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
        std::uniform_int_distribution<int> uniform(0, palit->second.size() - 1);
        int set = uniform(randomEngine);
        prim = data->vehicleColours[palit->second[set].first];
        sec = data->vehicleColours[palit->second[set].second];
    } else {
        logger->warning("World", "No colour palette for vehicle " + vti->name);
    }

    auto model = vti->getModel();
    auto info = data->vehicleInfo.find(vti->handling_);
    if (model && info != data->vehicleInfo.end() &&
            info->second->wheels.empty() && info->second->seats.empty()) {
        auto root = model->getFrame();
        for (const auto& frame : root->getChildren()) {
            const std::string& name = frame->getName();

            if (name.size() > 5 && name.substr(0, 5) == "wheel") {
                const auto& frameTrans = frame->getWorldTransform();
                info->second->wheels.push_back({glm::vec3(frameTrans[3])});
            }

            if (name == "chassis_dummy") {
                // These are nested within chassis_dummy
                auto frontseat = frame->findDescendant("ped_frontseat");
                auto backseat = frame->findDescendant("ped_backseat");
                if (frontseat) {
                    auto p = frontseat->getDefaultTranslation();
                    // Left seat
                    p.x = -p.x;
                    info->second->seats.front.push_back({p});
                    // Right seat
                    p.x = -p.x;
                    info->second->seats.front.push_back({p});
                }
                if (backseat) {
                    // @todo how does this work for the barracks, ambulance
                    // or coach?
                    auto p = backseat->getDefaultTranslation();
                    // Left seat
                    p.x = -p.x;
                    info->second->seats.back.push_back({p});
                    // Right seat
                    p.x = -p.x;
                    info->second->seats.back.push_back({p});
                }
            }
        }
    }

    auto vehicle =
        new VehicleObject{this, pos, rot, vti, info->second, prim, sec};
    vehicle->setGameObjectID(gid);

    vehiclePool.insert(vehicle);
    allObjects.push_back(vehicle);

    return vehicle;
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
    auto ped = new CharacterObject(this, pos, rot, pt, controller);
    ped->setGameObjectID(gid);
    pedestrianPool.insert(ped);
    allObjects.push_back(ped);
    return ped;
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
    auto ped = new CharacterObject(this, pos, rot, pt, controller);
    ped->setGameObjectID(gid);
    ped->setLifetime(GameObject::PlayerLifetime);
    players.push_back(controller);
    pedestrianPool.insert(ped);
    allObjects.push_back(ped);
    return ped;
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

    PickupObject* pickup = nullptr;
    auto pickuptype = (PickupObject::PickupType)type;

    auto it = std::find_if(
        data->weaponData.begin(), data->weaponData.end(),
        [=](const std::shared_ptr<WeaponData>& x) { return x->modelID == id; });

    // If nothing, create a generic pickup instead of an item pickup
    if (it != data->weaponData.end()) {
        pickup = new ItemPickup(this, pos, modelInfo, pickuptype, it->get());
    } else {
        RW_UNIMPLEMENTED("Non-weapon pickups");
        pickup = new PickupObject(this, pos, modelInfo, pickuptype);
    }

    pickupPool.insert(pickup);
    allObjects.push_back(pickup);

    return pickup;
}

void GameWorld::ObjectPool::insert(GameObject* object) {
    if (object->getGameObjectID() == 0) {
        // Find the lowest free GameObjectID.
        GameObjectID availID = 1;
        for (auto& p : objects) {
            if (p.first == availID) availID++;
        }

        object->setGameObjectID(availID);
    }
    objects[object->getGameObjectID()] = object;
}

GameObject* GameWorld::ObjectPool::find(GameObjectID id) const {
    auto it = objects.find(id);
    return (it == objects.end()) ? nullptr : it->second;
}

void GameWorld::ObjectPool::remove(GameObject* object) {
    if (object) {
        auto it = objects.find(object->getGameObjectID());
        if (it != objects.end()) {
            it = objects.erase(it);
        }
    }
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

    delete object;
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

VisualFX* GameWorld::createEffect(VisualFX::EffectType type) {
    auto effect = new VisualFX(type);
    effects.push_back(effect);
    return effect;
}

void GameWorld::destroyEffect(VisualFX* effect) {
    for (auto it = effects.begin(); it != effects.end();) {
        if (*it == effect) {
            it = effects.erase(it);
        } else {
            it++;
        }
    }
}

void GameWorld::doWeaponScan(const WeaponScan& scan) {
    RW_CHECK(scan.type != WeaponScan::RADIUS,
             "Radius scans not implemented yet");

    if (scan.type == WeaponScan::RADIUS) {
        // TODO
        // Requires custom ConvexResultCallback
    } else if (scan.type == WeaponScan::HITSCAN) {
        btVector3 from(scan.center.x, scan.center.y, scan.center.z),
            to(scan.end.x, scan.end.y, scan.end.z);
        glm::vec3 hitEnd = scan.end;
        btCollisionWorld::ClosestRayResultCallback cb(from, to);
        cb.m_collisionFilterGroup = btBroadphaseProxy::AllFilter;
        dynamicsWorld->rayTest(from, to, cb);
        // TODO: did any weapons penetrate?

        if (cb.hasHit()) {
            GameObject* go = static_cast<GameObject*>(
                cb.m_collisionObject->getUserPointer());
            GameObject::DamageInfo di;
            hitEnd = di.damageLocation =
                glm::vec3(cb.m_hitPointWorld.x(), cb.m_hitPointWorld.y(),
                          cb.m_hitPointWorld.z());
            di.damageSource = scan.center;
            di.type = GameObject::DamageInfo::Bullet;
            di.hitpoints = scan.damage;
            go->takeDamage(di);
        }
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

    object->takeDamage({{dmg.x(), dmg.y(), dmg.z()},
                        {src.x(), src.y(), src.z()},
                        0.f,
                        GameObject::DamageInfo::Physics,
                        mp.getAppliedImpulse()});
}

bool GameWorld::ContactProcessedCallback(btManifoldPoint& mp, void* body0,
                                         void* body1) {
    auto obA = static_cast<btCollisionObject*>(body0);
    auto obB = static_cast<btCollisionObject*>(body1);

    if (!(obA->getUserPointer() && obB->getUserPointer())) {
        return false;
    }

    GameObject* a = static_cast<GameObject*>(obA->getUserPointer());
    GameObject* b = static_cast<GameObject*>(obB->getUserPointer());

    bool aIsInstance = a && a->type() == GameObject::Instance;
    bool bIsInstance = b && b->type() == GameObject::Instance;

    bool exactly_one_gameobject_is_instance = aIsInstance != bIsInstance;

    if (exactly_one_gameobject_is_instance) {
        // Figure out which is the dynamic instance.
        InstanceObject* dynInst = nullptr;
        const btRigidBody *instBody = nullptr, *otherBody = nullptr;

        btVector3 src, dmg;

        if (aIsInstance) {
            dynInst = static_cast<InstanceObject*>(a);
            instBody = static_cast<const btRigidBody*>(obA);
            otherBody = static_cast<const btRigidBody*>(obB);
            src = mp.getPositionWorldOnB();
            dmg = mp.getPositionWorldOnA();
        } else {
            dynInst = static_cast<InstanceObject*>(b);
            instBody = static_cast<const btRigidBody*>(obB);
            otherBody = static_cast<const btRigidBody*>(obA);
            src = mp.getPositionWorldOnA();
            dmg = mp.getPositionWorldOnB();
        }

        if (dynInst->dynamics != nullptr && instBody->isStaticObject()) {
            // Attempt to determine relative velocity.
            auto dV = (otherBody->getLinearVelocity());
            auto impulse = dV.length() / (otherBody->getInvMass());

            if (dynInst->dynamics->uprootForce <= impulse) {
                dynInst->takeDamage({{dmg.x(), dmg.y(), dmg.z()},
                                     {src.x(), src.y(), src.z()},
                                     0.f,
                                     GameObject::DamageInfo::Physics,
                                     impulse});
            }
        }
    }

    // Handle vehicles
    if (a) handleVehicleResponse(a, mp, true);
    if (b) handleVehicleResponse(b, mp, false);

    return true;
}

void GameWorld::PhysicsTickCallback(btDynamicsWorld* physWorld,
                                    btScalar timeStep) {
    GameWorld* world = static_cast<GameWorld*>(physWorld->getWorldUserInfo());

    for (auto& p : world->vehiclePool.objects) {
        VehicleObject* object = static_cast<VehicleObject*>(p.second);
        object->tickPhysics(timeStep);
    }

    for (auto& p : world->pedestrianPool.objects) {
        CharacterObject* object = static_cast<CharacterObject*>(p.second);
        object->tickPhysics(timeStep);
    }
}

void GameWorld::loadCutscene(const std::string& name) {
    std::string lowerName(name);
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(),
                   ::tolower);

    auto datfile = data->index.openFile(lowerName + ".dat");

    CutsceneData* cutscene = new CutsceneData;

    if (datfile) {
        LoaderCutsceneDAT loaderdat;
        loaderdat.load(cutscene->tracks, datfile);
    }

    data->loadIFP(lowerName + ".ifp");

    cutsceneAudioLoaded = data->loadAudioStream(name + ".mp3");

    if (!cutsceneAudioLoaded) {
        cutsceneAudioLoaded = data->loadAudioStream(name + ".wav");
    }

    if (!cutsceneAudioLoaded) {
        logger->warning("Data", "Failed to load cutscene audio: " + name);
    }

    if (state->currentCutscene) {
        delete state->currentCutscene;
    }
    state->currentCutscene = cutscene;
    state->currentCutscene->meta.name = name;
    logger->info("World", "Loaded cutscene: " + name);
}

void GameWorld::startCutscene() {
    state->cutsceneStartTime = getGameTime();
    state->skipCutscene = false;

    if (cutsceneAudio.length() > 0) {
        sound.playMusic(cutsceneAudio);
    }
}

void GameWorld::clearCutscene() {
    for (auto& p : cutscenePool.objects) {
        destroyObjectQueued(p.second);
    }

    if (cutsceneAudio.length() > 0) {
        sound.stopMusic(cutsceneAudio);
        cutsceneAudio = "";
    }

    delete state->currentCutscene;
    state->currentCutscene = nullptr;
    state->isCinematic = false;
    state->cutsceneStartTime = -1.f;
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
    for (AIGraphNode* n : aigraph.nodes) {
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
    for (AIGraphNode* n : aigraph.nodes) {
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
    sound.pause(pause);
}

bool GameWorld::isPaused() const {
    return paused;
}

VehicleObject* GameWorld::tryToSpawnVehicle(VehicleGenerator& gen) {
    constexpr float kMinClearRadius = 10.f;

    if (gen.remainingSpawns <= 0) {
        return nullptr;
    }

    /// @todo take into account maxDelay as well
    if (gen.lastSpawnTime + gen.minDelay > int(state->basic.timeMS)) {
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
        auto info = data->vehicleInfo.find(model->handling_);
        if (info != data->vehicleInfo.end()) {
            const auto& handling = info->second->handling;
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
