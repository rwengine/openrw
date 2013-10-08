#include <renderwure/objects/GTACharacter.hpp>
#include <renderwure/ai/GTAAIController.hpp>
#include <renderwure/engine/GTAEngine.hpp>
#include <renderwure/engine/Animator.hpp>
#include <renderwure/objects/GTAVehicle.hpp>

GTACharacter::GTACharacter(GTAEngine* engine, const glm::vec3& pos, const glm::quat& rot, Model* model, std::shared_ptr<LoaderIDE::PEDS_t> ped)
: GTAObject(engine, pos, rot, model),
  currentVehicle(nullptr), ped(ped), physCharacter(nullptr),
  controller(nullptr), currentActivity(None)
{
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
		physCharacter = new btKinematicCharacterController(physObject, physShape, 3.5f, 2);
		
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
							GTAObject* object = static_cast<GTAObject*>(otherObject->getUserPointer());
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
		
		glm::vec3 direction = rotation * animator->getRootTranslation();
		physCharacter->setWalkDirection(btVector3(direction.x, direction.y, direction.z));
		
		btVector3 Pos = physCharacter->getGhostObject()->getWorldTransform().getOrigin();
		position = glm::vec3(Pos.x(), Pos.y(), Pos.z());
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

GTAVehicle *GTACharacter::getCurrentVehicle() const
{
	return currentVehicle;
}

void GTACharacter::setCurrentVehicle(GTAVehicle *value)
{
	currentVehicle = value;
	if(currentVehicle == nullptr && physCharacter == nullptr) {
		createActor();
	}
	else if(currentVehicle) {
		destroyActor();
	}
}
