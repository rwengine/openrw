#include <renderwure/engine/GTAEngine.hpp>
#include <renderwure/loaders/LoaderIPL.hpp>
#include <renderwure/loaders/LoaderIDE.hpp>
#include <renderwure/ai/GTADefaultAIController.hpp>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <renderwure/render/Model.hpp>

// 3 isn't enough to cause a factory.
#include <renderwure/objects/GTACharacter.hpp>
#include <renderwure/objects/GTAInstance.hpp>
#include <renderwure/objects/GTAVehicle.hpp>

GTAEngine::GTAEngine(const std::string& path)
    : itemCount(0), gameTime(0.f), gameData(path), renderer(this), randomEngine(rand())
{
	gameData.engine = this;
}

bool GTAEngine::load()
{
	collisionConfig = new btDefaultCollisionConfiguration;
	collisionDispatcher = new btCollisionDispatcher(collisionConfig);
	broadphase = new btDbvtBroadphase();
	solver = new btSequentialImpulseConstraintSolver;
	dynamicsWorld = new btDiscreteDynamicsWorld(collisionDispatcher, broadphase, solver, collisionConfig);
	dynamicsWorld->setGravity(btVector3(0.f, 0.f, -9.81f));
	broadphase->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
	
	gameData.load();

	return true;
}

void GTAEngine::logInfo(const std::string& info)
{
	log.push_back({LogEntry::Info, gameTime, info});
}

void GTAEngine::logError(const std::string& error)
{
	log.push_back({LogEntry::Error, gameTime, error});
}

void GTAEngine::logWarning(const std::string& warning)
{
	log.push_back({LogEntry::Warning, gameTime, warning});
}

bool GTAEngine::defineItems(const std::string& name)
{
	auto i = gameData.ideLocations.find(name);
	std::string path = name;
	
	if( i != gameData.ideLocations.end()) {
		path = i->second;
	}
	else {
		std::cout << "IDE not pre-listed" << std::endl;
	}
	
	LoaderIDE idel;
	
	if(idel.load(path)) {
		for( size_t o = 0; o < idel.OBJSs.size(); ++o) {
			objectTypes.insert({
				idel.OBJSs[o].ID, 
				std::shared_ptr<LoaderIDE::OBJS_t>(new LoaderIDE::OBJS_t(idel.OBJSs[o]))
			});
		}
		
		for( size_t v = 0; v < idel.CARSs.size(); ++v) {
			std::cout << "Vehicle ID " << idel.CARSs[v].ID << ": " << idel.CARSs[v].gameName << std::endl;
			vehicleTypes.insert({
				idel.CARSs[v].ID,
				std::shared_ptr<LoaderIDE::CARS_t>(new LoaderIDE::CARS_t(idel.CARSs[v]))
			});
		}

        for( size_t v = 0; v < idel.PEDSs.size(); ++v) {
            pedestrianTypes.insert({
                idel.PEDSs[v].ID,
                std::shared_ptr<LoaderIDE::PEDS_t>(new LoaderIDE::PEDS_t(idel.PEDSs[v]))
            });
        }

        // Load AI information.
        for( size_t a = 0; a < idel.PATHs.size(); ++a ) {
            auto pathit = objectNodes.find(idel.PATHs[a].ID);
            if( pathit == objectNodes.end() ) {
                objectNodes.insert({
                                   idel.PATHs[a].ID,
                                   {idel.PATHs[a]}
                               });
            }
            else {
                pathit->second.push_back(idel.PATHs[a]);
            }
        }
	}
	else {
		std::cerr << "Failed to load IDE " << path << std::endl;
	}
	
	return false;
}

bool GTAEngine::placeItems(const std::string& name)
{
	auto i = gameData.iplLocations.find(name);
	std::string path = name;
	
	if(i != gameData.iplLocations.end())
	{
		path = i->second;
	}
	else
	{
		std::cout << "IPL not pre-listed" << std::endl;
	}
	
	LoaderIPL ipll;

	if(ipll.load(path))
	{
		// Find the object.
		for( size_t i = 0; i < ipll.m_instances.size(); ++i) {
			LoaderIPLInstance& inst = ipll.m_instances[i];
			glm::quat rot(-inst.rotW, inst.rotX, inst.rotY, inst.rotZ);
			rot = glm::normalize(rot);
			if(! createInstance(inst.id, glm::vec3(inst.posX, inst.posY, inst.posZ), rot)) {
				std::cerr << "No object for instance " << inst.id << " (" << path << ")" << std::endl;
			}
		}
		itemCentroid += ipll.centroid;
		itemCount += ipll.m_instances.size();
		
		// Associate LODs.
		for( size_t i = 0; i < objectInstances.size(); ++i ) {
			if( !objectInstances[i]->object->LOD ) {
				auto lodInstit = modelInstances.find("LOD" + objectInstances[i]->object->modelName.substr(3));
				if( lodInstit != modelInstances.end() ) {
					objectInstances[i]->LODinstance = lodInstit->second;
				}
			}
		}
		
		return true;
	}
	else
	{
		std::cerr << "Failed to load IPL: " << path << std::endl;
		return false;
	}
	
	return false;
}

bool GTAEngine::loadZone(const std::string& path)
{
	LoaderIPL ipll;

	if( ipll.load(path)) {
		if( ipll.zones.size() > 0) {
			zones.insert(zones.begin(), ipll.zones.begin(), ipll.zones.end());
			std::cout << "Loaded " << ipll.zones.size() << " zones" << std::endl;
			return true;
		}
	}
	else {
		std::cerr << "Failed to load IPL " << path << std::endl;
	}
	
	return false;
}

GTAInstance *GTAEngine::createInstance(const uint16_t id, const glm::vec3& pos, const glm::quat& rot)
{
	auto oi = objectTypes.find(id);
	if( oi != objectTypes.end()) {
		// Make sure the DFF and TXD are loaded
		if(! oi->second->modelName.empty()) {
			gameData.loadDFF(oi->second->modelName + ".dff");
		}
		if(! oi->second->textureName.empty()) {
			gameData.loadTXD(oi->second->textureName + ".txd");
		}
		
		auto instance = std::shared_ptr<GTAInstance>(new GTAInstance(
			this,
			pos,
			rot,
			gameData.models[oi->second->modelName],
			glm::vec3(1.f, 1.f, 1.f),
			oi->second, nullptr
		));
		
		objectInstances.push_back(instance);
		
		if( !oi->second->modelName.empty() ) {
			modelInstances.insert({
				oi->second->modelName,
				objectInstances.back()
			});
		}

		return instance.get();
	}
	
	return nullptr;
}

GTAVehicle *GTAEngine::createVehicle(const uint16_t id, const glm::vec3& pos, const glm::quat& rot)
{
	auto vti = vehicleTypes.find(id);
	if(vti != vehicleTypes.end()) {
		std::cout << "Creating Vehicle ID " << id << " (" << vti->second->gameName << ")" << std::endl;
		
		if(! vti->second->modelName.empty()) {
			gameData.loadDFF(vti->second->modelName + ".dff");
		}
		if(! vti->second->textureName.empty()) {
			gameData.loadTXD(vti->second->textureName + ".txd");
		}
		
		glm::vec3 prim = glm::vec3(1.f), sec = glm::vec3(0.5f);
		auto palit = gameData.vehiclePalettes.find(vti->second->modelName); // modelname is conveniently lowercase (usually)
		if(palit != gameData.vehiclePalettes.end() && palit->second.size() > 0 ) {
			 std::uniform_int_distribution<int> uniform(0, palit->second.size()-1);
			 int set = uniform(randomEngine);
			 prim = gameData.vehicleColours[palit->second[set].first];
			 sec = gameData.vehicleColours[palit->second[set].second];
		}
		else {
			logWarning("No colour palette for vehicle " + vti->second->modelName);
		}
		
		auto wi = objectTypes.find(vti->second->wheelModelID);
		if( wi != objectTypes.end()) {
			if(! wi->second->textureName.empty()) {
				gameData.loadTXD(wi->second->textureName + ".txd");
			}
		}
		
		Model* model = gameData.models[vti->second->modelName];
		auto info = gameData.vehicleInfo.find(vti->second->handlingID);
		if(model && info != gameData.vehicleInfo.end()) {
			if( info->second.wheels.size() == 0 ) {
				for( size_t f = 0; f < model->frames.size(); ++f) {
					if( model->frameNames.size() > f) {
						std::string& name = model->frameNames[f];
						if( name.substr(0, 5) == "wheel" ) {
							auto frameTrans = model->getFrameMatrix(f);
							info->second.wheels.push_back({glm::vec3(frameTrans[3])});
						}
					}
				}
			}
		}
		
		vehicleInstances.push_back(new GTAVehicle{ this, pos, rot, model, vti->second, info->second, prim, sec });
		return vehicleInstances.back();
	}
	return nullptr;
}

GTACharacter* GTAEngine::createPedestrian(const uint16_t id, const glm::vec3 &pos, const glm::quat& rot)
{
    auto pti = pedestrianTypes.find(id);
    if( pti != pedestrianTypes.end() ) {
        auto& pt = pti->second;

        // Ensure the relevant data is loaded.
        if(! pt->modelName.empty()) {
			if( pt->modelName != "null" ) {
				gameData.loadDFF(pt->modelName + ".dff");
			}
        }
        if(! pt->textureName.empty()) {
            gameData.loadTXD(pt->textureName + ".txd");
        }

        Model* model = gameData.models[pt->modelName];

		if(model != nullptr) {
			auto ped = new GTACharacter( this, pos, rot, model, pt );
			pedestrians.push_back(ped);
			new GTADefaultAIController(ped);
			return ped;
		}
    }
    return nullptr;
}
