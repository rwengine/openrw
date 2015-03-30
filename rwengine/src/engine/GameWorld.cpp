#include <engine/GameWorld.hpp>

#include <core/Logger.hpp>

#include <loaders/LoaderIPL.hpp>
#include <loaders/LoaderIDE.hpp>
#include <ai/DefaultAIController.hpp>
#include <ai/TrafficDirector.hpp>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <render/Model.hpp>
#include <data/WeaponData.hpp>
#include <WorkContext.hpp>

#include <script/ScriptMachine.hpp>
#include <script/modules/VMModule.hpp>
#include <script/modules/GameModule.hpp>
#include <script/modules/ObjectModule.hpp>

// 3 isn't enough to cause a factory.
#include <objects/CharacterObject.hpp>
#include <objects/InstanceObject.hpp>
#include <objects/VehicleObject.hpp>
#include <objects/CutsceneObject.hpp>

#include <data/CutsceneData.hpp>
#include <loaders/LoaderCutsceneDAT.hpp>

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

GameWorld::GameWorld(Logger* log, const std::string& path)
	: logger(log), gameTime(0.f), gameData(log, path), randomEngine(rand()),
	  _work( new WorkContext( this ) ), script(nullptr), cutsceneAudio(nullptr), missionAudio(nullptr),
	  paused(false)
{
	gameData.engine = this;
	
	collisionConfig = new btDefaultCollisionConfiguration;
	collisionDispatcher = new WorldCollisionDispatcher(collisionConfig);
	broadphase = new btDbvtBroadphase();
	solver = new btSequentialImpulseConstraintSolver;
	dynamicsWorld = new btDiscreteDynamicsWorld(collisionDispatcher, broadphase, solver, collisionConfig);
	dynamicsWorld->setGravity(btVector3(0.f, 0.f, -9.81f));
	broadphase->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
	gContactProcessedCallback = ContactProcessedCallback;
	dynamicsWorld->setInternalTickCallback(PhysicsTickCallback, this);
}

GameWorld::~GameWorld()
{
	delete _work;

	for(auto o : objects) {
		delete o;
	}

	delete dynamicsWorld;
	delete solver;
	delete broadphase;
	delete collisionDispatcher;
	delete collisionConfig;

	/// @todo delete other things.
}

bool GameWorld::defineItems(const std::string& name)
{
	auto i = gameData.ideLocations.find(name);
	std::string path = name;

	LoaderIDE idel;
	
	if(idel.load(path)) {
		objectTypes.insert(idel.objects.begin(), idel.objects.end());

		// Load AI information.
		for( size_t a = 0; a < idel.PATHs.size(); ++a ) {
			auto pathit = objectNodes.find(idel.PATHs[a]->ID);
			if( pathit == objectNodes.end() ) {
					objectNodes.insert({
															idel.PATHs[a]->ID,
															{idel.PATHs[a]}
													});
			}
			else {
					pathit->second.push_back(idel.PATHs[a]);
			}
		}
	}
	else {
		logger->error("Data", "Failed to load IDE " + path);
	}
	
	return false;
}

void GameWorld::runScript(const std::string &name)
{
	SCMFile* f = gameData.loadSCM(name);
	if( f ) {
		if( script ) delete script;
		
		SCMOpcodes* opcodes = new SCMOpcodes;
		opcodes->modules.push_back(new VMModule);
		opcodes->modules.push_back(new GameModule);
		opcodes->modules.push_back(new ObjectModule);

		script = new ScriptMachine(this, f, opcodes);
	}
	else {
		logger->error("World", "Failed to load SCM: " + name);
	}
}

bool GameWorld::placeItems(const std::string& name)
{
	auto i = gameData.iplLocations.find(name);
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
		for(GameObject* object : objects) {
			if( object->type() == GameObject::Instance ) {
				InstanceObject* instance = static_cast<InstanceObject*>(object);
				if( !instance->object->LOD ) {
					auto lodInstit = modelInstances.find("LOD" + instance->object->modelName.substr(3));
					if( lodInstit != modelInstances.end() ) {
						instance->LODinstance = lodInstit->second;
					}
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
	auto oi = findObjectType<ObjectData>(id);
	if( oi ) {

		std::string modelname = oi->modelName;
		std::string texturename = oi->textureName;

		std::transform(std::begin(modelname), std::end(modelname), std::begin(modelname), tolower);
		std::transform(std::begin(texturename), std::end(texturename), std::begin(texturename), tolower);

		// Ensure the relevant data is loaded.
		if(! oi->modelName.empty()) {
			if( modelname != "null" ) {
				gameData.loadDFF(modelname + ".dff", true);
			}
		}
		if(! texturename.empty()) {
			gameData.loadTXD(texturename + ".txd", true);
		}

		ModelRef m = gameData.models[modelname];

		// Check for dynamic data.
		auto dyit = gameData.dynamicObjectData.find(oi->modelName);
		std::shared_ptr<DynamicObjectData> dydata;
		if( dyit != gameData.dynamicObjectData.end() ) {
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

		objects.insert(instance);

		modelInstances.insert({
			oi->modelName,
			instance
		});

		return instance;
	}
	
	return nullptr;
}

void GameWorld::createTraffic(const glm::vec3& near)
{
	TrafficDirector director(&aigraph, this);
	
	director.populateNearby( near, 100, 5 );
}

void GameWorld::cleanupTraffic(const glm::vec3& focus)
{
	for ( GameObject* object : objects )
	{
		if ( object->getLifetime() != GameObject::TrafficLifetime )
		{
			continue;
		}
		
		if ( glm::distance( focus, object->getPosition() ) >= 100.f )
		{
			destroyObjectQueued( object );
		}
	}
	destroyQueuedObjects();
}

#include <strings.h>
uint16_t GameWorld::findModelDefinition(const std::string model)
{
	// Dear C++ Why do I have to resort to strcasecmp this isn't C.
	auto defit = std::find_if(objectTypes.begin(), objectTypes.end(),
							  [&](const decltype(objectTypes)::value_type& d)
	{
		if(d.second->class_type == ObjectInformation::_class("OBJS"))
		{
			auto dat = static_cast<ObjectData*>(d.second.get());
			return strcasecmp(dat->modelName.c_str(), model.c_str()) == 0;
		}
		return false;
	});
	if( defit != objectTypes.end() ) return defit->first;
	return -1;
}

#include <ai/PlayerController.hpp>
#include <core/Logger.hpp>
CutsceneObject *GameWorld::createCutsceneObject(const uint16_t id, const glm::vec3 &pos, const glm::quat &rot)
{
	std::string modelname;
	std::string texturename;

	auto type = objectTypes.find(id);
	if( type != objectTypes.end() )
	{
		if( type->second->class_type == ObjectInformation::_class("HIER") )
		{
			modelname = state.specialModels[id];
			texturename = state.specialModels[id];
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
					modelname = state.specialCharacters[specialID];
					texturename = state.specialCharacters[specialID];
				}
			}
		}
	}

	if( id == 0 ) {
		modelname = state.player->getCharacter()->model->name;
	}

	// Ensure the relevant data is loaded.
	if( modelname.empty() ) {
		logger->error("World", "Cutscene object " + std::to_string(id) + " has no model");
		return nullptr;
	}

	if( modelname != "null" ) {
		gameData.loadDFF(modelname + ".dff", false);
	}

	if(! texturename.empty()) {
		gameData.loadTXD(texturename + ".txd", true);
	}


	ModelRef m = gameData.models[modelname];

	auto instance = new CutsceneObject(
		this,
		pos,
		m);

	objects.insert(instance);


	return instance;
}

VehicleObject *GameWorld::createVehicle(const uint16_t id, const glm::vec3& pos, const glm::quat& rot)
{
	auto vti = findObjectType<VehicleData>(id);
	if( vti ) {
		logger->info("World", "Creating Vehicle ID " + std::to_string(id) + " (" + vti->gameName + ")");
		
		if(! vti->modelName.empty()) {
			gameData.loadDFF(vti->modelName + ".dff");
		}
		if(! vti->textureName.empty()) {
			gameData.loadTXD(vti->textureName + ".txd");
		}
		
		glm::u8vec3 prim(255), sec(128);
		auto palit = gameData.vehiclePalettes.find(vti->modelName); // modelname is conveniently lowercase (usually)
		if(palit != gameData.vehiclePalettes.end() && palit->second.size() > 0 ) {
			 std::uniform_int_distribution<int> uniform(0, palit->second.size()-1);
			 int set = uniform(randomEngine);
			 prim = gameData.vehicleColours[palit->second[set].first];
			 sec = gameData.vehicleColours[palit->second[set].second];
		}
		else {
			logger->warning("World", "No colour palette for vehicle " + vti->modelName);
		}
		
		auto wi = findObjectType<ObjectData>(vti->wheelModelID);
		if( wi )
		{
			if(! wi->textureName.empty()) {
				gameData.loadTXD(wi->textureName + ".txd");
			}
		}
		
		ModelRef& m = gameData.models[vti->modelName];
		auto model = m->resource;
		auto info = gameData.vehicleInfo.find(vti->handlingID);
		if(model && info != gameData.vehicleInfo.end()) {
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

		objects.insert(vehicle);

		return vehicle;
	}
	return nullptr;
}

CharacterObject* GameWorld::createPedestrian(const uint16_t id, const glm::vec3 &pos, const glm::quat& rot)
{
	auto pt = findObjectType<CharacterData>(id);
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
				modelname = state.specialCharacters[specialID];
				texturename = state.specialCharacters[specialID];
			}

			if( modelname != "null" ) {
				gameData.loadDFF(modelname + ".dff");
			}
		}
		if(! texturename.empty()) {
			gameData.loadTXD(texturename + ".txd");
		}

		ModelRef m = gameData.models[modelname];

		if(m && m->resource) {
			auto ped = new CharacterObject( this, pos, rot, m, pt );
			objects.insert(ped);
			new DefaultAIController(ped);
			return ped;
		}
	}
	return nullptr;
}

void GameWorld::destroyObject(GameObject* object)
{
	auto iterator = objects.find(object);
	if( iterator != objects.end() ) {
		delete object;
		objects.erase(iterator);
	}
}

void GameWorld::destroyObjectQueued(GameObject *object)
{
	deletionQueue.push(object);
}

void GameWorld::destroyQueuedObjects()
{
	while( !deletionQueue.empty() ) {
		destroyObject( deletionQueue.front() );
		deletionQueue.pop();
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
	return state.hour;
}

int GameWorld::getMinute()
{
	return state.minute;
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

	for( GameObject* object : world->objects ) {
		if( object->type() == GameObject::Vehicle ) {
			static_cast<VehicleObject*>(object)->tickPhysics(timeStep);
		}
	}
}

void GameWorld::loadCutscene(const std::string &name)
{
	std::string lowerName(name);
	std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

	auto datfile = gameData.openFile(lowerName + ".dat");

	CutsceneData* cutscene = new CutsceneData;

	if( datfile ) {
		LoaderCutsceneDAT loaderdat;
		loaderdat.load(cutscene->tracks, datfile);
	}

	gameData.loadIFP(lowerName + ".ifp");

	cutsceneAudioLoaded = gameData.loadAudioStream(name+".mp3");
	
	if ( !cutsceneAudioLoaded )
	{
		cutsceneAudioLoaded = gameData.loadAudioStream(name+".wav");
	}
	
	if ( !cutsceneAudioLoaded )
	{
		logger->warning("Data", "Failed to load cutscene audio: " + name);
	}
	

	if( state.currentCutscene ) {
		delete state.currentCutscene;
	}
	state.currentCutscene = cutscene;
	state.currentCutscene->meta.name = name;
	logger->info("World", "Loaded cutscene: " + name);
}

void GameWorld::startCutscene()
{
	state.cutsceneStartTime = gameTime;
	state.skipCutscene = false;
	if( cutsceneAudio ) {
		cutsceneAudio->play();
	}
}

void GameWorld::clearCutscene()
{
	for(auto o : objects) {
		if( o->type() == GameObject::Cutscene ) {
			destroyObjectQueued(o);
		}
	}

	if( cutsceneAudio )
	{
		cutsceneAudio->stop();
		delete cutsceneAudio;
		cutsceneAudio = nullptr;
	}

	delete state.currentCutscene;
	state.currentCutscene = nullptr;
	state.isCinematic = false;
	state.cutsceneStartTime = -1.f;
}

bool GameWorld::isCutsceneDone()
{
	if( state.currentCutscene ) {
		float time = gameTime - state.cutsceneStartTime;
		if( state.skipCutscene ) {
			return true;
		}
		return time > state.currentCutscene->tracks.duration;
	}
	return true;
}


void GameWorld::loadSpecialCharacter(const unsigned short index, const std::string &name)
{
	std::string lowerName(name);
	std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
	/// @todo a bit more smarter than this
	state.specialCharacters[index] = lowerName;
}

void GameWorld::loadSpecialModel(const unsigned short index, const std::string &name)
{
	std::string lowerName(name);
	std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
	/// @todo a bit more smarter than this
	state.specialModels[index] = lowerName;
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

