#include <objects/PickupObject.hpp>
#include <objects/CharacterObject.hpp>
#include <engine/GameWorld.hpp>
#include <engine/GameData.hpp>

bool PickupObject::doesRespawn(PickupType type)
{
	switch(type) {
	case Once:
	case OnceTimeout:
	case Collectable:
	case OutOfStock:
	case Money:
	case MineInactive:
	case MineArmed:
	case NauticalMineInactive:
	case NauticalMineArmed:
	case FloatingPackage:
	case FloatingPackageFloating:
		return false;
	default:
		return true;
	}
}

float PickupObject::respawnTime(PickupType type)
{
	switch(type) {
	case InShop:
		return 5.f;
	case OnStreet:
	case OnStreetSlow:
		return 30.f;
	default:
		return 30.f;
	}
}

uint32_t PickupObject::behaviourFlags(PickupType type)
{
	switch(type) {
	case InShop:
	case OnStreet:
	case Once:
	case OnceTimeout:
	case Collectable:
	case Money:
	case OnStreetSlow:
		return PickupOnFoot;
	case OutOfStock:
	case MineInactive:
	case MineArmed:
	case NauticalMineInactive:
	case NauticalMineArmed:
		return 0;
	case FloatingPackage:
	case FloatingPackageFloating:
		return PickupInVehicle;
	default:
		return 0;
	}
}

PickupObject::PickupObject(GameWorld *world, const glm::vec3 &position, int modelID, PickupType type)
	: GameObject(world, position, glm::quat(), nullptr)
	, m_ghost(nullptr)
	, m_shape(nullptr)
	, m_enabled(false)
	, m_collected(false)
	, m_model(modelID)
	, m_type(type)
{
	btTransform tf;
	tf.setIdentity();
	tf.setOrigin(btVector3(position.x, position.y, position.z));

	m_ghost = new btPairCachingGhostObject;
	m_ghost->setUserPointer(this);
	m_ghost->setWorldTransform(tf);
	m_shape = new btSphereShape(0.5f);
	m_ghost->setCollisionShape(m_shape);
	m_ghost->setCollisionFlags(btCollisionObject::CF_KINEMATIC_OBJECT|btCollisionObject::CF_NO_CONTACT_RESPONSE);

	m_corona = world->createEffect(VisualFX::Particle);
	m_corona->particle.position = getPosition();
	m_corona->particle.direction = glm::vec3(0.f, 0.f, 1.f);
	m_corona->particle.orientation = VisualFX::ParticleData::Camera;
	m_corona->particle.colour = glm::vec4(1.0f, 0.3f, 0.3f, 0.3f);
	m_corona->particle.texture = engine->data->findTexture("coronacircle");

	auto flags = behaviourFlags(m_type);
	RW_CHECK((flags & PickupInVehicle)==0, "In Vehicle pickup not implemented yet");

	setEnabled(true);
}

PickupObject::~PickupObject()
{
	if(m_ghost) {
		setEnabled(false);
		engine->destroyEffect(m_corona);
		delete m_ghost;
		delete m_shape;
	}
}

void PickupObject::tick(float dt)
{
	if(! m_enabled) {
		// Check if our type of pickup respawns
		if (doesRespawn(m_type)) {
			m_enableTimer -= dt;
			if( m_enableTimer <= 0.f ) {
				setEnabled(true);
				m_collected = false;
			}
		}
	}

	if(m_enabled) {
		// Sort out interactions with things that may or may not be players.
		btManifoldArray manifoldArray;
		btBroadphasePairArray& pairArray = m_ghost->getOverlappingPairCache()->getOverlappingPairArray();
		int numPairs = pairArray.size();
		auto flags = behaviourFlags(m_type);

		for (int i=0;i<numPairs;i++)
		{
			manifoldArray.clear();

			const btBroadphasePair& pair = pairArray[i];
			auto otherObject = static_cast<const btCollisionObject*>(
						pair.m_pProxy0->m_clientObject == m_ghost ? pair.m_pProxy1->m_clientObject : pair.m_pProxy0->m_clientObject);
			if(otherObject->getUserPointer()) {
				GameObject* object = static_cast<GameObject*>(otherObject->getUserPointer());
				if((flags & PickupOnFoot) == PickupOnFoot
						&& object->type() == Character)
				{
					CharacterObject* character = static_cast<CharacterObject*>(object);
					m_collected = onCharacterTouch(character);
					setEnabled( !m_collected );

					if( ! m_enabled ) {
						m_enableTimer = respawnTime(m_type);
					}
				}
			}
		}
	}
}

void PickupObject::setEnabled(bool enabled)
{
	if( ! m_enabled && enabled ) {
		engine->dynamicsWorld->addCollisionObject(m_ghost, btBroadphaseProxy::SensorTrigger);
		m_corona->particle.size = glm::vec2(1.5f, 1.5f);
	}
	else if( m_enabled && ! enabled ) {
		engine->dynamicsWorld->removeCollisionObject(m_ghost);
		m_corona->particle.size = glm::vec2(0.f, 0.f);
	}

	m_enabled = enabled;
}
