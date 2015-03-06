#include <objects/PickupObject.hpp>
#include <objects/CharacterObject.hpp>
#include <engine/GameWorld.hpp>

PickupObject::PickupObject(GameWorld *world, const glm::vec3 &position, int modelID)
	: GameObject(world, position, glm::quat(), nullptr),
	  _ghost(nullptr), _shape(nullptr), _enabled(false), collected(false), _modelID(modelID)
{
	btTransform tf;
	tf.setIdentity();
	tf.setOrigin(btVector3(position.x, position.y, position.z));

	_ghost = new btPairCachingGhostObject;
	_ghost->setUserPointer(this);
	_ghost->setWorldTransform(tf);
	_shape = new btSphereShape(0.5f);
	_ghost->setCollisionShape(_shape);
	_ghost->setCollisionFlags(btCollisionObject::CF_KINEMATIC_OBJECT|btCollisionObject::CF_NO_CONTACT_RESPONSE);

	corona = world->createEffect(VisualFX::Particle);
	corona->particle.position = getPosition();
	corona->particle.direction = glm::vec3(0.f, 0.f, 1.f);
	corona->particle.orientation = VisualFX::ParticleData::Camera;
	corona->particle.colour = glm::vec4(1.0f, 0.3f, 0.3f, 0.3f);
	corona->particle.texture = engine->gameData.findTexture("coronacircle");

	setEnabled(true);
}

PickupObject::~PickupObject()
{
	if(_ghost) {
		setEnabled(false);
		engine->destroyEffect(corona);
		delete _ghost;
		delete _shape;
	}
}

void PickupObject::tick(float dt)
{
	if(! _enabled) {
		_enableTimer -= dt;
		if( _enableTimer <= 0.f ) {
			setEnabled(true);
			collected = false;
		}
	}

	if(_enabled) {
		// Sort out interactions with things that may or may not be players.
		btManifoldArray manifoldArray;
		btBroadphasePairArray& pairArray = _ghost->getOverlappingPairCache()->getOverlappingPairArray();
		int numPairs = pairArray.size();

		for (int i=0;i<numPairs;i++)
		{
			manifoldArray.clear();

			const btBroadphasePair& pair = pairArray[i];
			auto otherObject = static_cast<const btCollisionObject*>(
						pair.m_pProxy0->m_clientObject == _ghost ? pair.m_pProxy1->m_clientObject : pair.m_pProxy0->m_clientObject);
			if(otherObject->getUserPointer()) {
				GameObject* object = static_cast<GameObject*>(otherObject->getUserPointer());
				if(object->type() == Character) {
					CharacterObject* character = static_cast<CharacterObject*>(object);
					collected = onCharacterTouch(character);
					setEnabled( !collected );
					if( ! _enabled ) {
						_enableTimer = 60.f;
					}
				}
			}
		}
	}
}

void PickupObject::setEnabled(bool enabled)
{
	if( ! _enabled && enabled ) {
		engine->dynamicsWorld->addCollisionObject(_ghost, btBroadphaseProxy::SensorTrigger);
		corona->particle.size = glm::vec2(1.5f, 1.5f);
	}
	else if( _enabled && ! enabled ) {
		engine->dynamicsWorld->removeCollisionObject(_ghost);
		corona->particle.size = glm::vec2(0.f, 0.f);
	}

	_enabled = enabled;
}
