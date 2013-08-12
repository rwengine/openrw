#include <renderwure/engine/GTAEngine.hpp>
#include <renderwure/loaders/LoaderIPL.hpp>
#include <renderwure/loaders/LoaderIDE.hpp>
#include <renderwure/ai/GTADefaultAIController.hpp>

GTAEngine::GTAEngine(const std::string& path)
    : renderer(this), itemCount(0), gameData(path), gameTime(0.f), randomEngine(rand())
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
	broadphase->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());
	
	gameData.load();
	
	// Loade all of the IDEs.
	for(std::map<std::string, std::string>::iterator it = gameData.ideLocations.begin();
		it != gameData.ideLocations.end();
		++it) {
		defineItems(it->second);
	}
	
	// Load the .zon IPLs since we want to have the zones loaded
	for(std::map<std::string, std::string>::iterator it = gameData.iplLocations.begin();
		it != gameData.iplLocations.end();
		++it) {
		loadZone(it->second);
		placeItems(it->second);
	}
	
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
			auto oi = objectTypes.find(inst.id);
			if( oi != objectTypes.end()) {
				// Make sure the DFF and TXD are loaded
				if(! oi->second->modelName.empty()) {
					gameData.loadDFF(oi->second->modelName + ".dff");
				}
				if(! oi->second->textureName.empty()) {
					gameData.loadTXD(oi->second->textureName + ".txd");
				}
				
                static size_t bodycount = 0;

				btRigidBody* body = nullptr;
				auto phyit = gameData.collisions.find(oi->second->modelName);
                if( phyit != gameData.collisions.end()) {
					btCompoundShape* cmpShape = new btCompoundShape;
					btDefaultMotionState* msta = new btDefaultMotionState;
					msta->setWorldTransform(btTransform(
						btQuaternion(
							inst.rotX, inst.rotY, inst.rotZ, inst.rotW
						), 
						btVector3(
							inst.posX, inst.posY, inst.posZ
						)
					));
					btRigidBody::btRigidBodyConstructionInfo info(0.f, msta, cmpShape);
					CollisionInstance& physInst = *phyit->second.get();
					
					// Boxes
					for( size_t i = 0; i < physInst.boxes.size(); ++i ) {
						CollTBox& box = physInst.boxes[i];
						auto size = (box.max - box.min) / 2.f;
						auto mid = (box.min + box.max) / 2.f;
						btCollisionShape* bshape = new btBoxShape( btVector3(size.x, size.y, size.z)  );
						btTransform t(btQuaternion(0.f, 0.f, 0.f, 1.f), btVector3(mid.x, mid.y, mid.z));
						cmpShape->addChildShape(t, bshape);
                        bodycount++;
					}
					
					// Spheres
					for( size_t i = 0; i < physInst.spheres.size(); ++i ) {
						CollTSphere& sphere = physInst.spheres[i];
						btCollisionShape* sshape = new btSphereShape(sphere.radius);
						btTransform t(btQuaternion(0.f, 0.f, 0.f, 1.f), btVector3(sphere.center.x, sphere.center.y, sphere.center.z));
						cmpShape->addChildShape(t, sshape);
                        bodycount++;
					}

                    if( physInst.triangles.size() > 0 ) {
                        btTriangleIndexVertexArray* vertarray = new btTriangleIndexVertexArray(
                                    physInst.triangles.size(),
                                    (int*) physInst.triangles.data(),
                                    sizeof(CollTFaceTriangle),
                                    physInst.vertices.size(),
                                    &(physInst.vertices[0].x),
                                sizeof(glm::vec3)
                                );
                        btBvhTriangleMeshShape* trishape = new btBvhTriangleMeshShape(vertarray, false);
                        cmpShape->addChildShape(
                                    btTransform(btQuaternion(0.f, 0.f, 0.f, 1.f), btVector3(0.f, 0.f, 0.f)),
                                    trishape
                                    );
                        bodycount++;
                    }

					
					// Todo: other shapes.
					
					body = new btRigidBody(info);
					//body->setWorldTransform();
					dynamicsWorld->addRigidBody(body);
				}

                glm::vec3 instancePos(inst.posX, inst.posY, inst.posZ);
                glm::quat instanceRot(-inst.rotW, inst.rotX, inst.rotY, inst.rotZ);
                instanceRot = glm::normalize(instanceRot);

                auto pathit = objectNodes.find(inst.id);
                if( pathit != objectNodes.end() ) {
                    auto& pathlist = pathit->second;
                    for( size_t p = 0; p < pathlist.size(); ++p ) {
                        auto& path = pathlist[p];
                        size_t startIndex = ainodes.size();
                        for( size_t n = 0; n < path.nodes.size(); ++n ) {
                            auto& node = path.nodes[n];

                            GTAAINode::NodeType type = (path.type == LoaderIDE::PATH_PED ? GTAAINode::Pedestrian : GTAAINode::Vehicle);
                            int32_t next = node.next >= 0 ? startIndex + node.next : -1;
                            uint32_t flags = GTAAINode::None;
                            glm::vec3 position = instancePos + (instanceRot * node.position);

                            if( node.type == LoaderIDE::EXTERNAL ) {
                                flags |= GTAAINode::External;
                                for( size_t rn = 0; rn < ainodes.size(); ++rn ) {
                                    if( (ainodes[rn].flags & GTAAINode::External) == GTAAINode::External ) {
                                        auto d = glm::length(ainodes[rn].position - position);
                                        if( d < 1.f ) {
                                            next = rn;
                                            break;
                                        }
                                    }
                                }
                            }

                            ainodes.push_back({
                                                  type,
                                                  position,
                                                  flags,
                                                  next
                                              });
                        }
                    }
                }

				
                objectInstances.push_back({
                                              this,
                                              instancePos,
                                              instanceRot,
                                              gameData.models[inst.model],
                                              glm::vec3(inst.scaleX, inst.scaleY, inst.scaleZ),
                                              inst, oi->second
                                          });
			}
			else {
				std::cerr << "No object for instance " << inst.id << " (" << path << ")" << std::endl;
			}
		}
		itemCentroid += ipll.centroid;
		itemCount += ipll.m_instances.size();
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

void GTAEngine::createVehicle(const uint16_t id, const glm::vec3& pos, const glm::quat& rot)
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
		if(model) {
			if( vti->second->wheelPositions.size() == 0 ) {
				for( size_t f = 0; f < model->frames.size(); ++f) {
					if( model->frameNames.size() > f) {
						std::string& name = model->frameNames[f];
						if( name.substr(0, 5) == "wheel" ) {
                            vti->second->wheelPositions.push_back(model->frames[f].defaultTranslation);
						}
					}
				}
			}
		}
		
        vehicleInstances.push_back({ this, pos, rot, model, vti->second, prim, sec });
	}
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
            else {
                gameData.loadDFF("loplyguy.dff");
            }
        }
        if(! pt->textureName.empty()) {
            gameData.loadTXD(pt->textureName + ".txd");
        }

        Model* model = gameData.models[pt->modelName];

		auto ped = new GTACharacter( this, pos, rot, model, pt );
		pedestrians.push_back(ped);
		ped->changeAction(GTACharacter::Idle);
		new GTADefaultAIController(ped);
		dynamicsWorld->addCollisionObject(ped->physObject, btBroadphaseProxy::CharacterFilter, btBroadphaseProxy::StaticFilter|btBroadphaseProxy::DefaultFilter);
		dynamicsWorld->addAction(ped->physCharacter);
		return ped;
    }
    return nullptr;
}
