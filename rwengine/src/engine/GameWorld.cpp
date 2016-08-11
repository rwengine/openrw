#include <engine/GameWorld.hpp>
#include <engine/GameData.hpp>
#include <engine/GameState.hpp>

#include <core/Logger.hpp>

#include <loaders/LoaderIPL.hpp>
#include <loaders/LoaderIDE.hpp>
#include <ai/DefaultAIController.hpp>
#include <ai/TrafficDirector.hpp>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <data/Model.hpp>
#include <data/WeaponData.hpp>
#include <job/WorkContext.hpp>
#include <items/WeaponItem.hpp>

// 3 isn't enough to cause a factory.
#include <objects/CharacterObject.hpp>
#include <objects/InstanceObject.hpp>
#include <objects/VehicleObject.hpp>
#include <objects/CutsceneObject.hpp>
#include <objects/ItemPickup.hpp>

#include <data/CutsceneData.hpp>
#include <loaders/LoaderCutsceneDAT.hpp>
#ifdef RW_WINDOWS
#include <rw_mingw.hpp>
#endif

#include <render/ViewCamera.hpp>

// Behaviour Tuning
constexpr float kMaxTrafficSpawnRadius = 100.f;
constexpr float kMaxTrafficCleanupRadius = kMaxTrafficSpawnRadius * 1.25f;

class WorldCollisionDispatcher : public btCollisionDispatcher
{
public:

	WorldCollisionDispatcher(btCollisionConfiguration* collisionConfiguration)
		: btCollisionDispatcher(collisionConfiguration)
	{}

	bool needsResponse(const btCollisionObject *obA, const btCollisionObject *obB) {
		if( !( obA->getUserPointer() && obB->getUserPointer() ) ) {
			return btCollisionDispatcher::needsResponse(obA, obB);
		}

		GameObject* a = static_cast<GameObject*>(obA->getUserPointer());
		GameObject* b = static_cast<GameObject*>(obB->getUserPointer());

		bool valA = a && a->type() == GameObject::Instance;
		bool valB = b && b->type() == GameObject::Instance;

		if( ! (valA && valB) &&	(valB || valA) ) {

			// Figure out which is the dynamic instance.
			InstanceObject* dynInst = nullptr;
			const btRigidBody* instBody = nullptr, * otherBody = nullptr;

			if( valA ) {
				dynInst = static_cast<InstanceObject*>(a);
				instBody = static_cast<const btRigidBody*>(obA);
				otherBody = static_cast<const btRigidBody*>(obB);
			}
			else {
				dynInst = static_cast<InstanceObject*>(b);
				instBody = static_cast<const btRigidBody*>(obB);
				otherBody = static_cast<const btRigidBody*>(obA);
			}

			if( dynInst->dynamics == nullptr || ! instBody->isStaticObject() ) {
				return btCollisionDispatcher::needsResponse(obA, obB);
			}

			// Attempt to determine relative velocity.
			auto dV  = (otherBody->getLinearVelocity());
			auto impulse = dV.length();

			// Ignore collision if the object is about to be uprooted.
			if(	dynInst->dynamics->uprootForce <= impulse / (otherBody->getInvMass()) ) {
				return false;
			}
		}
		return btCollisionDispatcher::needsResponse(obA, obB);
	}
};

GameWorld::GameWorld(Logger* log, WorkContext* work, GameData* dat)
	: logger(log), data(dat), randomEngine(rand()),
	  _work( work ),
	  paused(false)
{
	data->engine = this;
	
	collisionConfig = new btDefaultCollisionConfiguration;
	collisionDispatcher = new WorldCollisionDispatcher(collisionConfig);
	broadphase = new btDbvtBroadphase();
	solver = new btSequentialImpulseConstraintSolver;
	dynamicsWorld = new btDiscreteDynamicsWorld(collisionDispatcher, broadphase, solver, collisionConfig);
	dynamicsWorld->setGravity(btVector3(0.f, 0.f, -9.81f));
	broadphase->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
	gContactProcessedCallback = ContactProcessedCallback;
	dynamicsWorld->setInternalTickCallback(PhysicsTickCallback, this);

	// Populate inventory items
	for( auto& w : data->weaponData ) {
		inventoryItems.push_back(
					new WeaponItem(
						inventoryItems.size(),
						w));
	}
}

GameWorld::~GameWorld()
{
	for(auto& p : allObjects) {
		delete p;
	}

	delete dynamicsWorld;
	delete solver;
	delete broadphase;
	delete collisionDispatcher;
	delete collisionConfig;

	/// @todo delete other things.
}

bool GameWorld::placeItems(const std::string& name)
{
	std::string path = name;
	
	LoaderIPL ipll;

	if(ipll.load(path))
	{
		// Find the object.
		for( size_t i = 0; i < ipll.m_instances.size(); ++i) {
			std::shared_ptr<InstanceData> inst = ipll.m_instances[i];
			if(! createInstance(inst->id, inst->pos, inst->rot)) {
				logger->error("World", "No object data for instance " + std::to_string(inst->id) + " in " + path);
			}
		}
		
		// Attempt to Associate LODs.
		for(auto& p: instancePool.objects) {
			auto object = p.second;
			InstanceObject* instance = static_cast<InstanceObject*>(object);
			if( !instance->object->LOD ) {
				auto lodInstit = modelInstances.find("LOD" + instance->object->modelName.substr(3));
				if( lodInstit != modelInstances.end() ) {
					instance->LODinstance = lodInstit->second;
				}
			}
		}
		
		return true;
	}
	else
	{
		logger->error("Data", "Failed to load IPL " + path);
		return false;
	}
	
	return false;
}

InstanceObject *GameWorld::createInstance(const uint16_t id, const glm::vec3& pos, const glm::quat& rot)
{
	auto oi = data->findObjectType<ObjectData>(id);
	if( oi ) {

		std::string modelname = oi->modelName;
		std::string texturename = oi->textureName;

		std::transform(std::begin(modelname), std::end(modelname), std::begin(modelname), tolower);
		std::transform(std::begin(texturename), std::end(texturename), std::begin(texturename), tolower);

		// Ensure the relevant data is loaded.
		if(! oi->modelName.empty()) {
			if( modelname != "null" ) {
				data->loadDFF(modelname + ".dff", false);
			}
		}
		if(! texturename.empty()) {
			data->loadTXD(texturename + ".txd", true);
		}

		ModelRef m = data->models[modelname];

		// Check for dynamic data.
		auto dyit = data->dynamicObjectData.find(oi->modelName);
		std::shared_ptr<DynamicObjectData> dydata;
		if( dyit != data->dynamicObjectData.end() ) {
			dydata = dyit->second;
		}

		if( modelname.empty() ) {
			logger->warning("World", "Instance with missing model: " + std::to_string(id));
		}
		
		auto instance = new InstanceObject(
			this,
			pos,
			rot,
			m,
			glm::vec3(1.f, 1.f, 1.f),
			oi, nullptr, dydata
		);

		instancePool.insert(instance);
        allObjects.push_back(instance);

		modelInstances.insert({
			oi->modelName,
			instance
		});

		return instance;
	}
	
	return nullptr;
}

void GameWorld::createTraffic(const ViewCamera& viewCamera)
{
	TrafficDirector director(&aigraph, this);
	
	director.populateNearby( viewCamera, kMaxTrafficSpawnRadius, 5 );
}

void GameWorld::cleanupTraffic(const ViewCamera& focus)
{
	for (auto& p : pedestrianPool.objects) {
		if (p.second->getLifetime() != GameObject::TrafficLifetime) {
			continue;
		}
		
		if (glm::distance( focus.position, p.second->getPosition() ) >= kMaxTrafficCleanupRadius) {
			if (! focus.frustum.intersects(p.second->getPosition(), 1.f)) {
				destroyObjectQueued( p.second );
			}
		}
	}
	for ( auto& p : vehiclePool.objects ) {
		if (p.second->getLifetime() != GameObject::TrafficLifetime) {
			continue;
		}

		if (glm::distance( focus.position, p.second->getPosition() ) >= kMaxTrafficCleanupRadius) {
			if (! focus.frustum.intersects(p.second->getPosition(), 1.f)) {
				destroyObjectQueued( p.second );
			}
		}
	}

	destroyQueuedObjects();
}

#include <ai/PlayerController.hpp>
#include <core/Logger.hpp>
CutsceneObject *GameWorld::createCutsceneObject(const uint16_t id, const glm::vec3 &pos, const glm::quat &rot)
{
	std::string modelname;
	std::string texturename;

	auto type = data->objectTypes.find(id);
	if( type != data->objectTypes.end() )
	{
		if( type->second->class_type == ObjectInformation::_class("HIER") )
		{
			modelname = state->specialModels[id];
			texturename = state->specialModels[id];
		}
		else
		{
			if( type->second->class_type == ObjectInformation::_class("OBJS") )
			{
				auto v = static_cast<ObjectData*>(type->second.get());
				modelname = v->modelName;
				texturename = v->textureName;
			}
			else if( type->second->class_type == ObjectInformation::_class("PEDS") )
			{
				auto v = static_cast<CharacterData*>(type->second.get());
				modelname = v->modelName;
				texturename = v->textureName;


				static std::string specialPrefix("special");
				if(! modelname.compare(0, specialPrefix.size(), specialPrefix) ) {
					auto sid = modelname.substr(specialPrefix.size());
					unsigned short specialID = std::atoi(sid.c_str());
					modelname = state->specialCharacters[specialID];
					texturename = state->specialCharacters[specialID];
				}
			}
		}
	}

	if( id == 0 ) {
		auto playerobj = pedestrianPool.find(state->playerObject);
		if( playerobj )
		{
			modelname = playerobj->model->name;
		}
	}

	// Ensure the relevant data is loaded.
	if( modelname.empty() ) {
		logger->error("World", "Cutscene object " + std::to_string(id) + " has no model");
		return nullptr;
	}

	if( modelname != "null" ) {
		data->loadDFF(modelname + ".dff", false);
	}

	if(! texturename.empty()) {
		data->loadTXD(texturename + ".txd", true);
	}


	ModelRef m = data->models[modelname];

	auto instance = new CutsceneObject(
		this,
		pos,
		rot,
		m);

	cutscenePool.insert( instance );
    allObjects.push_back(instance);


	return instance;
}

VehicleObject *GameWorld::createVehicle(const uint16_t id, const glm::vec3& pos, const glm::quat& rot, GameObjectID gid)
{
	auto vti = data->findObjectType<VehicleData>(id);
	if( vti ) {
		logger->info("World", "Creating Vehicle ID " + std::to_string(id) + " (" + vti->gameName + ")");
		
		if(! vti->modelName.empty()) {
			data->loadDFF(vti->modelName + ".dff");
		}
		if(! vti->textureName.empty()) {
			data->loadTXD(vti->textureName + ".txd");
		}
		
		glm::u8vec3 prim(255), sec(128);
		auto palit = data->vehiclePalettes.find(vti->modelName); // modelname is conveniently lowercase (usually)
		if(palit != data->vehiclePalettes.end() && palit->second.size() > 0 ) {
			 std::uniform_int_distribution<int> uniform(0, palit->second.size()-1);
			 int set = uniform(randomEngine);
			 prim = data->vehicleColours[palit->second[set].first];
			 sec = data->vehicleColours[palit->second[set].second];
		}
		else {
			logger->warning("World", "No colour palette for vehicle " + vti->modelName);
		}
		
		auto wi = data->findObjectType<ObjectData>(vti->wheelModelID);
		if( wi )
		{
			if(! wi->textureName.empty()) {
				data->loadTXD(wi->textureName + ".txd");
			}
		}
		
		ModelRef& m = data->models[vti->modelName];
		auto model = m->resource;
		auto info = data->vehicleInfo.find(vti->handlingID);
		if(model && info != data->vehicleInfo.end()) {
			if( info->second->wheels.size() == 0 && info->second->seats.size() == 0 ) {
				for( const ModelFrame* f : model->frames ) {
					const std::string& name = f->getName();
					
					if( name.size() > 5 && name.substr(0, 5) == "wheel" ) {
						auto frameTrans = f->getMatrix();
						info->second->wheels.push_back({glm::vec3(frameTrans[3])});
					}
					if(name.size() > 3 && name.substr(0, 3) == "ped" && name.substr(name.size()-4) == "seat") {
						auto p = f->getDefaultTranslation();
						p.x = p.x * -1.f;
						info->second->seats.push_back({p});
						p.x = p.x * -1.f;
						info->second->seats.push_back({p});
					}
				}
			}
		}

		auto vehicle = new VehicleObject{ this, pos, rot, m, vti, info->second, prim, sec };
		vehicle->setGameObjectID(gid);

		vehiclePool.insert( vehicle );
        allObjects.push_back( vehicle );

		return vehicle;
	}
	return nullptr;
}

CharacterObject* GameWorld::createPedestrian(const uint16_t id, const glm::vec3& pos, const glm::quat& rot, GameObjectID gid)
{
	auto pt = data->findObjectType<CharacterData>(id);
	if( pt ) {

		std::string modelname = pt->modelName;
		std::string texturename = pt->textureName;

		// Ensure the relevant data is loaded.
		if(! pt->modelName.empty()) {
			// Some model names have special meanings.
			/// @todo Should CharacterObjects handle this?
			static std::string specialPrefix("special");
			if(! modelname.compare(0, specialPrefix.size(), specialPrefix) ) {
				auto sid = modelname.substr(specialPrefix.size());
				unsigned short specialID = std::atoi(sid.c_str());
				modelname = state->specialCharacters[specialID];
				texturename = state->specialCharacters[specialID];
			}

			if( modelname != "null" ) {
				data->loadDFF(modelname + ".dff");
			}
		}
		if(! texturename.empty()) {
			data->loadTXD(texturename + ".txd");
		}

		ModelRef m = data->models[modelname];

		if(m && m->resource) {
			auto ped = new CharacterObject( this, pos, rot, m, pt );
			ped->setGameObjectID(gid);
			new DefaultAIController(ped);
			pedestrianPool.insert( ped );
            allObjects.push_back( ped );
			return ped;
		}
	}
	return nullptr;
}

CharacterObject* GameWorld::createPlayer(const glm::vec3& pos, const glm::quat& rot, GameObjectID gid)
{
	// Player object ID is hardcoded to 0.
	auto pt = data->findObjectType<CharacterData>(0);
	if( pt ) {
		// Model name is also hardcoded.
		std::string modelname = "player";
		std::string texturename = "player";

		// Ensure the relevant data is loaded.
		data->loadDFF(modelname + ".dff");
		data->loadTXD(texturename + ".txd");

		ModelRef m = data->models[modelname];

		if(m && m->resource) {
			auto ped = new CharacterObject( this, pos, rot, m, nullptr );
			ped->setGameObjectID(gid);
			ped->setLifetime(GameObject::PlayerLifetime);
			players.push_back(new PlayerController(ped));
			pedestrianPool.insert(ped);
            allObjects.push_back( ped );
			return ped;
		}
	}
	return nullptr;
}

PickupObject* GameWorld::createPickup(const glm::vec3& pos, int id, int type)
{
	auto modelInfo = data->findObjectType<ObjectData>(id);

	RW_CHECK(modelInfo != nullptr, "Pickup Object Data is not found");
	if (modelInfo == nullptr) {
		return nullptr;
	}

	data->loadDFF(modelInfo->modelName + ".dff");
	data->loadTXD(modelInfo->textureName + ".txd");

	PickupObject* pickup = nullptr;
	auto pickuptype = (PickupObject::PickupType)type;

	// Attempt to find an InventoryItem associated with this model
	auto it = std::find_if(
	    inventoryItems.begin(), inventoryItems.end(),
	    [=](InventoryItem* itm) { return itm->getModelID() == id; });

	// If nothing, create a generic pickup instead of an item pickup
	if (it != inventoryItems.end()) {
		pickup = new ItemPickup(this, pos, pickuptype, *it);
	}
	else {
		RW_UNIMPLEMENTED("Non-weapon pickups");
		pickup = new PickupObject(this, pos, id, pickuptype);
	}

	pickupPool.insert(pickup);
	allObjects.push_back(pickup);

	return pickup;
}

void GameWorld::ObjectPool::insert(GameObject* object)
{
	if( object->getGameObjectID() == 0 )
	{
		// Find the lowest free GameObjectID.
		GameObjectID availID = 1;
		for( auto& p : objects )
		{
			if( p.first == availID ) availID++;
		}

		object->setGameObjectID( availID );
	}
	objects[object->getGameObjectID()] = object;
}

GameObject* GameWorld::ObjectPool::find(GameObjectID id) const
{
	auto it = objects.find( id );
	return (it == objects.end())? nullptr : it->second;
}

void GameWorld::ObjectPool::remove(GameObject* object)
{
	if( object )
	{
		auto it = objects.find(object->getGameObjectID());
		if( it != objects.end() ) {
			it = objects.erase(it);
		}
	}
}


GameWorld::ObjectPool& GameWorld::getTypeObjectPool(GameObject* object)
{
	switch( object->type() ) {
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

GameObject*GameWorld::getBlipTarget(const BlipData& blip) const
{
	switch( blip.type )
	{
		case BlipData::Vehicle:
			return vehiclePool.find(blip.target);
		case BlipData::Character:
			return pedestrianPool.find(blip.target);
		case BlipData::Pickup:
			return pickupPool.find(blip.target);
		default:
			return nullptr;
	}
}

void GameWorld::destroyObject(GameObject* object)
{
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

void GameWorld::destroyObjectQueued(GameObject *object)
{
	RW_CHECK(object != nullptr, "destroying a null object?");
	if (object)
		deletionQueue.insert(object);
}

void GameWorld::destroyQueuedObjects()
{
	while( !deletionQueue.empty() ) {
		destroyObject( *deletionQueue.begin() );
		deletionQueue.erase( deletionQueue.begin() );
	}
}

VisualFX* GameWorld::createEffect(VisualFX::EffectType type)
{
	auto effect = new VisualFX( type );
	effects.push_back(effect);
	return effect;
}

void GameWorld::destroyEffect(VisualFX* effect)
{
	for( auto it = effects.begin(); it != effects.end(); )
	{
		if( *it == effect )
		{
			it = effects.erase( it );
		}
		else
		{
			it++;
		}
	}
}

void GameWorld::doWeaponScan(const WeaponScan &scan)
{
	RW_CHECK(scan.type != WeaponScan::RADIUS, "Radius scans not implemented yet");

	if( scan.type == WeaponScan::RADIUS ) {
		// TODO
		// Requires custom ConvexResultCallback
	}
	else if( scan.type == WeaponScan::HITSCAN ) {
		btVector3 from(scan.center.x, scan.center.y, scan.center.z),
				to(scan.end.x, scan.end.y, scan.end.z);
		glm::vec3 hitEnd = scan.end;
		btCollisionWorld::ClosestRayResultCallback cb(from, to);
		cb.m_collisionFilterGroup = btBroadphaseProxy::AllFilter;
		dynamicsWorld->rayTest(from, to, cb);
		// TODO: did any weapons penetrate?

		if( cb.hasHit() ) {
			GameObject* go = static_cast<GameObject*>(cb.m_collisionObject->getUserPointer());
			GameObject::DamageInfo di;
			hitEnd = di.damageLocation = glm::vec3(cb.m_hitPointWorld.x(),
								  cb.m_hitPointWorld.y(),
								  cb.m_hitPointWorld.z() );
			di.damageSource = scan.center;
			di.type = GameObject::DamageInfo::Bullet;
			di.hitpoints = scan.damage;
			go->takeDamage(di);
		}
	}
}

int GameWorld::getHour()
{
	return state->basic.gameHour;
}

int GameWorld::getMinute()
{
	return state->basic.gameMinute;
}

glm::vec3 GameWorld::getGroundAtPosition(const glm::vec3 &pos) const
{
	btVector3 rayFrom(pos.x, pos.y, 100.f);
	btVector3 rayTo(pos.x, pos.y, -100.f);

	btDynamicsWorld::ClosestRayResultCallback rr(rayFrom, rayTo);

	dynamicsWorld->rayTest( rayFrom, rayTo, rr );

	if(rr.hasHit()) {
		auto& ws = rr.m_hitPointWorld;
		return { ws.x(), ws.y(), ws.z() };
	}

	return pos;
}

float GameWorld::getGameTime() const
{
	return state->gameTime;
}

InventoryItem* GameWorld::getInventoryItem(uint16_t weaponId) const
{
	RW_CHECK(weaponId < inventoryItems.size(), "InventoryItem ID out of range");
	if (weaponId >= inventoryItems.size()) {
		return nullptr;
	}
	return inventoryItems[weaponId];
}

void handleVehicleResponse(GameObject* object, btManifoldPoint& mp, bool isA)
{
	bool isVehicle = object->type() == GameObject::Vehicle;
	if(! isVehicle) return;
	if( mp.getAppliedImpulse() <= 100.f ) return;

	btVector3 src, dmg;
	if(isA) {
		src = mp.getPositionWorldOnB();
		dmg = mp.getPositionWorldOnA();
	}
	else {
		src = mp.getPositionWorldOnA();
		dmg = mp.getPositionWorldOnB();
	}

	object->takeDamage({
							{dmg.x(), dmg.y(), dmg.z()},
							{src.x(), src.y(), src.z()},
							0.f,
							GameObject::DamageInfo::Physics,
							mp.getAppliedImpulse()
						});
}

bool GameWorld::ContactProcessedCallback(btManifoldPoint &mp, void *body0, void *body1)
{
	auto obA = static_cast<btCollisionObject*>(body0);
	auto obB = static_cast<btCollisionObject*>(body1);

	if( !( obA->getUserPointer() && obB->getUserPointer() ) ) {
		return false;
	}

	GameObject* a = static_cast<GameObject*>(obA->getUserPointer());
	GameObject* b = static_cast<GameObject*>(obB->getUserPointer());

	bool valA = a && a->type() == GameObject::Instance;
	bool valB = b && b->type() == GameObject::Instance;

	if( ! (valA && valB) &&	(valB || valA) ) {

		// Figure out which is the dynamic instance.
		InstanceObject* dynInst = nullptr;
		const btRigidBody* instBody = nullptr, * otherBody = nullptr;

		btVector3 src, dmg;

		if( valA ) {
			dynInst = static_cast<InstanceObject*>(a);
			instBody = static_cast<const btRigidBody*>(obA);
			otherBody = static_cast<const btRigidBody*>(obB);
			src = mp.getPositionWorldOnB();
			dmg = mp.getPositionWorldOnA();
		}
		else {
			dynInst = static_cast<InstanceObject*>(b);
			instBody = static_cast<const btRigidBody*>(obB);
			otherBody = static_cast<const btRigidBody*>(obA);
			src = mp.getPositionWorldOnA();
			dmg = mp.getPositionWorldOnB();
		}

		if( dynInst->dynamics != nullptr && instBody->isStaticObject() ) {
			// Attempt to determine relative velocity.
			auto dV  = (otherBody->getLinearVelocity());
			auto impulse = dV.length()/ (otherBody->getInvMass());

			if( dynInst->dynamics->uprootForce <= impulse ) {
				dynInst->takeDamage({
										{dmg.x(), dmg.y(), dmg.z()},
										{src.x(), src.y(), src.z()},
										0.f,
										GameObject::DamageInfo::Physics,
										impulse
									});
			}
		}
	}

	// Handle vehicles
	if(a) handleVehicleResponse(a, mp, true);
	if(b) handleVehicleResponse(b, mp, false);

	return true;
}

void GameWorld::PhysicsTickCallback(btDynamicsWorld *physWorld, btScalar timeStep)
{
	GameWorld* world = static_cast<GameWorld*>(physWorld->getWorldUserInfo());

	for( auto& p : world->vehiclePool.objects ) {
		GameObject* object = p.second;
		static_cast<VehicleObject*>(object)->tickPhysics(timeStep);
	}
}

void GameWorld::loadCutscene(const std::string &name)
{
	std::string lowerName(name);
	std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

	auto datfile = data->openFile(lowerName + ".dat");

	CutsceneData* cutscene = new CutsceneData;

	if( datfile ) {
		LoaderCutsceneDAT loaderdat;
		loaderdat.load(cutscene->tracks, datfile);
	}

	data->loadIFP(lowerName + ".ifp");

	cutsceneAudioLoaded = data->loadAudioStream(name+".mp3");
	
	if ( !cutsceneAudioLoaded )
	{
		cutsceneAudioLoaded = data->loadAudioStream(name+".wav");
	}
	
	if ( !cutsceneAudioLoaded )
	{
		logger->warning("Data", "Failed to load cutscene audio: " + name);
	}
	

	if( state->currentCutscene ) {
		delete state->currentCutscene;
	}
	state->currentCutscene = cutscene;
	state->currentCutscene->meta.name = name;
	logger->info("World", "Loaded cutscene: " + name);
}

void GameWorld::startCutscene()
{
	state->cutsceneStartTime = getGameTime();
	state->skipCutscene = false;

	if (cutsceneAudio.length() > 0) {
		sound.playMusic(cutsceneAudio);
	}
}

void GameWorld::clearCutscene()
{
	for(auto& p : cutscenePool.objects) {
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

bool GameWorld::isCutsceneDone()
{
	if( state->currentCutscene ) {
		float time = getGameTime() - state->cutsceneStartTime;
		if( state->skipCutscene ) {
			return true;
		}
		return time > state->currentCutscene->tracks.duration;
	}
	return true;
}


void GameWorld::loadSpecialCharacter(const unsigned short index, const std::string &name)
{
	std::string lowerName(name);
	std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
	/// @todo a bit more smarter than this
	state->specialCharacters[index] = lowerName;
	data->loadDFF(lowerName + ".dff");
}

void GameWorld::loadSpecialModel(const unsigned short index, const std::string &name)
{
	std::string lowerName(name);
	std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
	/// @todo a bit more smarter than this
	state->specialModels[index] = lowerName;
}

void GameWorld::disableAIPaths(AIGraphNode::NodeType type, const glm::vec3& min, const glm::vec3& max)
{
	for(AIGraphNode* n : aigraph.nodes)
	{
		if( n->type == type )
		{
			if( n->position.x >= min.x &&
				n->position.y >= min.y &&
				n->position.z >= min.z &&
				n->position.x <= max.x &&
				n->position.y <= max.y &&
				n->position.z <= max.z
			)
			{
				n->disabled = true;
			}
		}
	}
}

void GameWorld::enableAIPaths(AIGraphNode::NodeType type, const glm::vec3& min, const glm::vec3& max)
{
	for(AIGraphNode* n : aigraph.nodes)
	{
		if( n->type == type )
		{
			if( n->position.x >= min.x &&
				n->position.y >= min.y &&
				n->position.z >= min.z &&
				n->position.x <= max.x &&
				n->position.y <= max.y &&
				n->position.z <= max.z
			)
			{
				n->disabled = false;
			}
		}
	}
}

void GameWorld::drawAreaIndicator(AreaIndicatorInfo::AreaIndicatorType type, glm::vec3 position, glm::vec3 radius)
{
	areaIndicators.push_back({type, position, radius});
}

void GameWorld::clearTickData()
{
	areaIndicators.clear();
}

void GameWorld::setPaused(bool pause)
{
	paused = pause;
	sound.pause(pause);
}

bool GameWorld::isPaused() const
{
	return paused;
}

VehicleObject* GameWorld::tryToSpawnVehicle(VehicleGenerator& gen)
{
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
	for (auto& v : vehiclePool.objects)
	{
		if (glm::distance2(position, v.second->getPosition())
				< kMinClearRadius * kMinClearRadius) {
			return nullptr;
		}
	}

	int id = gen.vehicleID;
	if (id == -1) {
		// Random ID found by dice roll
		id = 134;
		/// @todo use zone information to decide vehicle id
	}

	auto vehicle = createVehicle(
				id,
				position);
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

