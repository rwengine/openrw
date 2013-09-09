#include <renderwure/objects/GTACharacter.hpp>
#include <renderwure/ai/GTAAIController.hpp>
#include <renderwure/engine/GTAEngine.hpp>

GTACharacter::GTACharacter(GTAEngine* engine, const glm::vec3& pos, const glm::quat& rot, Model* model, std::shared_ptr<LoaderIDE::PEDS_t> ped)
: GTAObject(engine, pos, rot, model), ped(ped), currentActivity(None), controller(nullptr)
{
	btTransform tf;
	tf.setIdentity();
	tf.setOrigin(btVector3(pos.x, pos.y, pos.z));

	physObject = new btPairCachingGhostObject;
	physObject->setWorldTransform(tf);
	physShape = new btBoxShape(btVector3(0.25f, 0.25f, 1.f));
	physObject->setCollisionShape(physShape);
	physObject->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);
	physCharacter = new btKinematicCharacterController(physObject, physShape, 0.65f, 2);

	engine->dynamicsWorld->addCollisionObject(physObject, btBroadphaseProxy::CharacterFilter, btBroadphaseProxy::StaticFilter|btBroadphaseProxy::DefaultFilter);
	engine->dynamicsWorld->addAction(physCharacter);

	changeAction(Idle);
}

void GTACharacter::changeAction(Activity newAction)
{
	if(currentActivity != newAction) {
		currentActivity = newAction;
		switch( currentActivity ) {
		case Idle:
			if( animation == nullptr || animation->name != "idle_stance" ) {
				animation = engine->gameData.animations.at("idle_stance");
				animtime = 0.f;
			}
			break;
		case Walk:
			if( animation == nullptr || animation->name != "walk_civi" ) {
				animation = engine->gameData.animations.at("walk_civi");
				animtime = 0.f;
			}
			break;
		case Run:
			if( animation == nullptr || animation->name != "run_civi" ) {
				animation = engine->gameData.animations.at("run_civi");
				animtime = 0.f;
			}
			break;
		}
	}
}

void GTACharacter::updateCharacter()
{
	if( controller ) {
		rotation = glm::normalize(controller->getTargetRotation());
	}

	glm::vec3 direction = rotation * (rootPosition - lastRootPosition);
	physCharacter->setWalkDirection(btVector3(direction.x, direction.y, direction.z));

	btVector3 Pos = physCharacter->getGhostObject()->getWorldTransform().getOrigin();
	position = glm::vec3(Pos.x(), Pos.y(), Pos.z());
}

void GTACharacter::updateAnimation(float dt)
{
	if( animation != nullptr ) {
		animtime += dt;
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
	btVector3 Pos = physCharacter->getGhostObject()->getWorldTransform().getOrigin();
	return glm::vec3(Pos.x(), Pos.y(), Pos.z());
}
