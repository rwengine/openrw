#include <objects/GTACharacter.hpp>
#include <ai/GTAAIController.hpp>
#include <engine/GameWorld.hpp>
#include <engine/Animator.hpp>
#include <objects/GTAVehicle.hpp>
#include <boost/concept_check.hpp>

GTACharacter::GTACharacter(GameWorld* engine, const glm::vec3& pos, const glm::quat& rot, Model* model, std::shared_ptr<CharacterData> data)
: GameObject(engine, pos, rot, model),
  currentVehicle(nullptr), currentSeat(0), ped(data), physCharacter(nullptr),
  controller(nullptr), currentActivity(None)
{
	mHealth = 100.f;
	if(model) {
		animator = new Animator();
		animator->setModel(model);

		createActor();
		changeAction(Idle);
	}
}

GTACharacter::~GTACharacter()
{
	destroyActor();
}

void GTACharacter::createActor(const glm::vec3& size)
{
	if(physCharacter) {
		destroyActor();
	}
	
	// Don't create anything without a valid model.
	if(model) {
		btTransform tf;
		tf.setIdentity();
		tf.setOrigin(btVector3(position.x, position.y, position.z));

		physObject = new btPairCachingGhostObject;
		physObject->setUserPointer(this);
		physObject->setWorldTransform(tf);
		physShape = new btCapsuleShapeZ(size.x, size.z);
		physObject->setCollisionShape(physShape);
		physObject->setCollisionFlags(btCollisionObject::CF_KINEMATIC_OBJECT);
		physCharacter = new btKinematicCharacterController(physObject, physShape, 0.2f, 2);
		physCharacter->setVelocityForTimeInterval(btVector3(1.f, 1.f, 0.f), 1.f);
		
		engine->dynamicsWorld->addCollisionObject(physObject, btBroadphaseProxy::KinematicFilter, btBroadphaseProxy::StaticFilter);
		engine->dynamicsWorld->addAction(physCharacter);
	}
}

void GTACharacter::destroyActor()
{
	if(physCharacter) {
		engine->dynamicsWorld->removeCollisionObject(physObject);
		engine->dynamicsWorld->removeAction(physCharacter);

		delete physCharacter;
		delete physObject;
		delete physShape;
		physCharacter = nullptr;
	}
}

void GTACharacter::changeAction(Activity newAction)
{
	if(currentActivity != newAction) {
		currentActivity = newAction;
		if(currentVehicle == nullptr) {
			switch( currentActivity ) {
			default:
			case Idle:
				animator->setAnimation(engine->gameData.animations.at("idle_stance"));
				break;
			case Walk:
				animator->setAnimation(engine->gameData.animations.at("walk_civi"));
				break;
			case Run:
				animator->setAnimation(engine->gameData.animations.at("run_civi"));
				break;
			case KnockedDown:
				// Change body shape.
				position += glm::vec3(0.f, 0.f, 0.5f);
				createActor(glm::vec3(0.5f, 0.5f, 0.1f));
				animator->setAnimation(engine->gameData.animations.at("kd_front"), false);
				break;
			case GettingUp:
				// Change body shape back to normal.
				createActor();
				animator->setAnimation(engine->gameData.animations.at("getup"), false);
				break;
			case VehicleDrive:
				animator->setAnimation(engine->gameData.animations.at("car_sit"));
				break;
			}
		}
		else {
			animator->setAnimation(engine->gameData.animations.at("car_sit"));
		}
	}
}

void GTACharacter::tick(float dt)
{
	if(controller) {
		controller->update(dt);
	}
	animator->tick(dt);
	updateCharacter();
	
	// Ensure the character doesn't need to be reset
	if(getPosition().z < -100.f) {
		resetToAINode();
	}
}

void GTACharacter::updateCharacter()
{
	if(physCharacter) {
		// Check to see if the character should be knocked down.
		btManifoldArray   manifoldArray;
		btBroadphasePairArray& pairArray = physObject->getOverlappingPairCache()->getOverlappingPairArray();
		int numPairs = pairArray.size();
	
		for (int i=0;i<numPairs;i++)
		{
			manifoldArray.clear();
			
			const btBroadphasePair& pair = pairArray[i];
	
			//unless we manually perform collision detection on this pair, the contacts are in the dynamics world paircache:
			btBroadphasePair* collisionPair = engine->dynamicsWorld->getPairCache()->findPair(pair.m_pProxy0,pair.m_pProxy1);
			if (!collisionPair)
				continue;
	
			if (collisionPair->m_algorithm)
				collisionPair->m_algorithm->getAllContactManifolds(manifoldArray);
	
			for (int j=0;j<manifoldArray.size();j++)
			{
				btPersistentManifold* manifold = manifoldArray[j];
				for (int p=0;p<manifold->getNumContacts();p++)
				{
					const btManifoldPoint&pt = manifold->getContactPoint(p);
					if (pt.getDistance() < 0.f)
					{
						auto otherObject = static_cast<const btCollisionObject*>(
							manifold->getBody0() == physObject ? manifold->getBody1() : manifold->getBody0());
						if(otherObject->getUserPointer()) {
							GameObject* object = static_cast<GameObject*>(otherObject->getUserPointer());
							if(object->type() == Vehicle) {
								GTAVehicle* vehicle = static_cast<GTAVehicle*>(object);
								if(vehicle->physBody->getLinearVelocity().length() > 0.1f) {
									changeAction(KnockedDown);
								}
							}
						}
					}
				}
			}
		}
		
		if(currentActivity == GTACharacter::Jump)
		{
			if(physCharacter->onGround())
			{
				changeAction(GTACharacter::Idle);
			}
		}
		else 
		{
			glm::vec3 direction = rotation * animator->getRootTranslation();
			physCharacter->setWalkDirection(btVector3(direction.x, direction.y, direction.z));
			
			btVector3 Pos = physCharacter->getGhostObject()->getWorldTransform().getOrigin();
			position = glm::vec3(Pos.x(), Pos.y(), Pos.z());
		}
	}
}

void GTACharacter::setPosition(const glm::vec3& pos)
{
	btTransform& tf = physCharacter->getGhostObject()->getWorldTransform();
	tf.setOrigin(btVector3(pos.x, pos.y, pos.z));
	position = pos;
}

glm::vec3 GTACharacter::getPosition() const
{
	if(physCharacter) {
		btVector3 Pos = physCharacter->getGhostObject()->getWorldTransform().getOrigin();
		return glm::vec3(Pos.x(), Pos.y(), Pos.z());
	}
	if(currentVehicle) {
		return currentVehicle->getPosition();
	}
	return position;
}

bool GTACharacter::enterVehicle(GTAVehicle* vehicle, size_t seat)
{
	if(vehicle) {
		// Check that the seat is free
		if(vehicle->getOccupant(seat)) {
			return false;
		}
		else {
			// Make sure we leave any vehicle we're inside
			enterVehicle(nullptr, 0);
			vehicle->setOccupant(seat, this);
			setCurrentVehicle(vehicle, seat);
			return true;
		}
	}
	else {
		if(currentVehicle) {
			currentVehicle->setOccupant(seat, nullptr);
			// Disabled due to crashing.
			//setPosition(currentVehicle->getPosition()); 
			setCurrentVehicle(nullptr, 0);
			return true;
		}
	}
	return false;
}

GTAVehicle *GTACharacter::getCurrentVehicle() const
{
	return currentVehicle;
}

size_t GTACharacter::getCurrentSeat() const
{
	return currentSeat;
}

void GTACharacter::setCurrentVehicle(GTAVehicle *value, size_t seat)
{
	currentVehicle = value;
	currentSeat = seat;
	if(currentVehicle == nullptr && physCharacter == nullptr) {
		createActor();
	}
	else if(currentVehicle) {
		destroyActor();
	}
}

bool GTACharacter::takeDamage(const GameObject::DamageInfo& dmg)
{
	mHealth -= dmg.hitpoints;
	return true;
}

void GTACharacter::jump()
{
	physCharacter->jump();
	changeAction(GTACharacter::Jump);
}

void GTACharacter::resetToAINode()
{
	auto nodes = engine->aigraph.nodes;
	bool vehicleNode = !! getCurrentVehicle();
	GTAAINode* nearest = nullptr; float d = std::numeric_limits<float>::max();
	for(auto it = nodes.begin(); it != nodes.end(); ++it) {
		if(vehicleNode) {
			if((*it)->type == GTAAINode::Pedestrian) continue;
		}
		else {
			if((*it)->type == GTAAINode::Vehicle) continue;
		}
		
		float dist = glm::length((*it)->position - getPosition());
		if(dist < d) {
			nearest = *it;
			d = dist;
		}
	}
	
	if(nearest) {
		if(vehicleNode) {
			getCurrentVehicle()->setPosition(nearest->position + glm::vec3(0.f, 0.f, 2.5f));
		}
		else {
			setPosition(nearest->position + glm::vec3(0.f, 0.f, 2.5f));
		}
	}
}

