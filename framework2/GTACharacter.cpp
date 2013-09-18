#include <renderwure/objects/GTACharacter.hpp>
#include <renderwure/ai/GTAAIController.hpp>
#include <renderwure/engine/GTAEngine.hpp>
#include <renderwure/engine/Animator.hpp>
#include <renderwure/objects/GTAVehicle.hpp>

GTACharacter::GTACharacter(GTAEngine* engine, const glm::vec3& pos, const glm::quat& rot, Model* model, std::shared_ptr<LoaderIDE::PEDS_t> ped)
: GTAObject(engine, pos, rot, model),
  currentVehicle(nullptr),
  ped(ped), currentActivity(None), controller(nullptr)
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

void GTACharacter::createActor()
{
	// Don't create anything without a valid model.
	if(model) {
		btTransform tf;
		tf.setIdentity();
		tf.setOrigin(btVector3(position.x, position.y, position.z));

		physObject = new btPairCachingGhostObject;
		physObject->setWorldTransform(tf);
		physShape = new btBoxShape(btVector3(0.25f, 0.25f, 1.f));
		physObject->setCollisionShape(physShape);
		physObject->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);
		physCharacter = new btKinematicCharacterController(physObject, physShape, 0.65f, 2);

		engine->dynamicsWorld->addCollisionObject(physObject, btBroadphaseProxy::CharacterFilter, btBroadphaseProxy::StaticFilter|btBroadphaseProxy::DefaultFilter);
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
