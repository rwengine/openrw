#include <engine/GameData.hpp>
#include <engine/GameWorld.hpp>
#include <objects/CharacterObject.hpp>
#include <objects/PickupObject.hpp>

uint32_t colours[14] = {
    0xff0000, // bat, detonator, adrenaline
    0x00ff00, // pistol
    0x8080ff, // uzi
    0xffff00, // shotgun
    0xff00ff, // ak47
    0x00ffff, // m16
    0xff8000, // sniper
    0x00ff80, // rocket
    0x8000ff, // flame
    0x80ff00, // molotov
    0xffffff, // grenade
    0x80ff80, // bodyarmour, bribe
    0x0000ff, // info, killfrenzy
    0xffff00  // health, bonus
};

bool PickupObject::doesRespawn(PickupType type) {
    switch (type) {
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

float PickupObject::respawnTime(PickupType type) {
    switch (type) {
        case InShop:
            return 5.f;
        case OnStreet:
        case OnStreetSlow:
            return 30.f;
        default:
            return 30.f;
    }
}

uint32_t PickupObject::behaviourFlags(PickupType type) {
    switch (type) {
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

PickupObject::PickupObject(GameWorld* world, const glm::vec3& position,
                           BaseModelInfo* modelinfo, PickupType type)
    : GameObject(world, position, glm::quat(), modelinfo)
    , m_ghost(nullptr)
    , m_shape(nullptr)
    , m_enabled(false)
    , m_collected(false)
    , m_type(type) {
    btTransform tf;
    tf.setIdentity();
    tf.setOrigin(btVector3(position.x, position.y, position.z));

    m_ghost = new btPairCachingGhostObject;
    m_ghost->setUserPointer(this);
    m_ghost->setWorldTransform(tf);
    m_shape = new btSphereShape(0.5f);
    m_ghost->setCollisionShape(m_shape);
    m_ghost->setCollisionFlags(btCollisionObject::CF_KINEMATIC_OBJECT |
                               btCollisionObject::CF_NO_CONTACT_RESPONSE);

    m_colourId = 0;
    if (modelinfo->name == "colt45")
        m_colourId = 1;
    else if (modelinfo->name == "uzi")
        m_colourId = 2;
    else if (modelinfo->name == "shotgun")
        m_colourId = 3;
    else if (modelinfo->name == "ak47")
        m_colourId = 4;
    else if (modelinfo->name == "m16")
        m_colourId = 5;
    else if (modelinfo->name == "sniper")
        m_colourId = 6;
    else if (modelinfo->name == "rocket")
        m_colourId = 7;
    else if (modelinfo->name == "flame")
        m_colourId = 8;
    else if (modelinfo->name == "molotov")
        m_colourId = 9;
    else if (modelinfo->name == "grenade")
        m_colourId = 10;
    else if (modelinfo->name == "bodyarmour" || modelinfo->name == "bribe")
        m_colourId = 11;
    else if (modelinfo->name == "info" || modelinfo->name == "killfrenzy")
        m_colourId = 12;
    else if (modelinfo->name == "health" || modelinfo->name == "bonus")
        m_colourId = 13;

    m_corona = world->createEffect(VisualFX::Particle);
    m_corona->particle.position = getPosition();
    m_corona->particle.direction = glm::vec3(0.f, 0.f, 1.f);
    m_corona->particle.orientation = VisualFX::ParticleData::Camera;
    m_corona->particle.texture = engine->data->findSlotTexture("particle", "coronaringa");

    auto flags = behaviourFlags(m_type);
    RW_UNUSED(flags);

    RW_CHECK((flags & PickupInVehicle) == 0,
             "In Vehicle pickup not implemented yet");

    setEnabled(true);
}

PickupObject::~PickupObject() {
    if (m_ghost) {
        setEnabled(false);
        engine->destroyEffect(m_corona);
        delete m_corona;
        delete m_ghost;
        delete m_shape;
    }
}

void PickupObject::tick(float dt) {
    if (!m_enabled) {
        // Check if our type of pickup respawns
        if (doesRespawn(m_type)) {
            m_enableTimer -= dt;
            if (m_enableTimer <= 0.f) {
                setEnabled(true);
                m_collected = false;
            }
        }
    }

    float time = engine->getGameTime();
    float colourValue = 0.5f * (sin(time * 3.0664064f) * 0.3f + 0.3f);
    uint32_t *colour = &colours[m_colourId];
    float red = (*colour >> 16) & 0xFF;
    float green = (*colour >> 8) & 0xFF;
    float blue = *colour & 0xFF;
    m_corona->particle.colour = glm::vec4(red / 255.f,
                                          green / 255.f,
                                          blue / 255.f,
                                          1.f) * colourValue;

    if (m_enabled) {
        // Sort out interactions with things that may or may not be players.
        btManifoldArray manifoldArray;
        btBroadphasePairArray& pairArray =
            m_ghost->getOverlappingPairCache()->getOverlappingPairArray();
        int numPairs = pairArray.size();
        auto flags = behaviourFlags(m_type);

        for (int i = 0; i < numPairs; i++) {
            manifoldArray.clear();

            const btBroadphasePair& pair = pairArray[i];
            auto otherObject = static_cast<const btCollisionObject*>(
                pair.m_pProxy0->m_clientObject == m_ghost
                    ? pair.m_pProxy1->m_clientObject
                    : pair.m_pProxy0->m_clientObject);
            if (otherObject->getUserPointer()) {
                GameObject* object =
                    static_cast<GameObject*>(otherObject->getUserPointer());
                if ((flags & PickupOnFoot) == PickupOnFoot &&
                    object->type() == Character) {
                    CharacterObject* character =
                        static_cast<CharacterObject*>(object);
                    m_collected = onCharacterTouch(character);
                    setEnabled(!m_collected);

                    if (!m_enabled) {
                        m_enableTimer = respawnTime(m_type);
                    }
                }
            }
        }
    }
}

void PickupObject::setEnabled(bool enabled) {
    if (!m_enabled && enabled) {
        engine->dynamicsWorld->addCollisionObject(
            m_ghost, btBroadphaseProxy::SensorTrigger);
        m_corona->particle.size = glm::vec2(1.5f, 1.5f);
    } else if (m_enabled && !enabled) {
        engine->dynamicsWorld->removeCollisionObject(m_ghost);
        m_corona->particle.size = glm::vec2(0.f, 0.f);
    }

    m_enabled = enabled;
}
