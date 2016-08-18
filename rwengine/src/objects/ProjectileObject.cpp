#include <objects/ProjectileObject.hpp>
#include <engine/GameWorld.hpp>
#include <engine/GameData.hpp>
#include <data/WeaponData.hpp>

void ProjectileObject::checkPhysicsContact()
{
  btManifoldArray manifoldArray;
  btBroadphasePairArray& pairArray =
      _ghostBody->getOverlappingPairCache()->getOverlappingPairArray();
  int numPairs = pairArray.size();

  for (int i = 0; i < numPairs; i++) {
    manifoldArray.clear();

    const btBroadphasePair& pair = pairArray[i];

    // unless we manually perform collision detection on this pair, the contacts are in the dynamics
    // world paircache:
    btBroadphasePair* collisionPair =
        engine->dynamicsWorld->getPairCache()->findPair(pair.m_pProxy0, pair.m_pProxy1);
    if (!collisionPair) {
      continue;
    }

    if (collisionPair->m_algorithm) {
      collisionPair->m_algorithm->getAllContactManifolds(manifoldArray);
    }

    for (int j = 0; j < manifoldArray.size(); j++) {
      // btPersistentManifold* manifold = manifoldArray[j];
      // const btCollisionObject* B = manifold->getBody0() == _ghostBody ? manifold->getBody1() :
      // manifold->getBody0();
      // GameObject* go = static_cast<GameObject*>(B->getUserPointer());

      /// @todo check if this is a suitable level to check c.f btManifoldPoint
      // It's happening
      explode();

      return;
    }
  }
}

void ProjectileObject::explode()
{
  if (!_exploded) {
    // Remove our physics objects
    cleanup();

    const float exp_size = 10.f;
    const float damageSize = 5.f;
    const float damage = _info.weapon->damage;

    for (auto& o : engine->allObjects) {
      if (o == this) continue;
      switch (o->type()) {
        case GameObject::Instance:
        case GameObject::Vehicle:
        case GameObject::Character:
          break;
        default:
          continue;
      }

      float d = glm::distance(getPosition(), o->getPosition());
      if (d > damageSize) continue;

      o->takeDamage(
          {getPosition(), getPosition(), damage / glm::max(d, 1.f), DamageInfo::Explosion, 0.f});
    }

    auto tex = engine->data->findTexture("explo02");

    auto explosion = engine->createEffect(VisualFX::Particle);
    explosion->particle.size = glm::vec2(exp_size);
    explosion->particle.texture = tex;
    explosion->particle.starttime = engine->getGameTime();
    explosion->particle.lifetime = 0.5f;
    explosion->particle.orientation = VisualFX::ParticleData::Camera;
    explosion->particle.colour = glm::vec4(1.0f);
    explosion->particle.position = getPosition();
    explosion->particle.direction = glm::vec3(0.f, 0.f, 1.f);

    _exploded = true;
    engine->destroyObjectQueued(this);
  }
}

void ProjectileObject::cleanup()
{
  if (_body) {
    engine->dynamicsWorld->removeRigidBody(_body);
    delete _body;
    _body = nullptr;
  }
  if (_ghostBody) {
    engine->dynamicsWorld->removeCollisionObject(_ghostBody);
    delete _ghostBody;
    _ghostBody = nullptr;
  }
  if (_shape) {
    delete _shape;
    _shape = nullptr;
  }
}

ProjectileObject::ProjectileObject(GameWorld* world, const glm::vec3& position,
                                   const ProjectileObject::ProjectileInfo& info)
    : GameObject(world, position, glm::quat(), nullptr),
      _info(info),
      _body(nullptr),
      _ghostBody(nullptr),
      _exploded(false)
{
  _shape = new btSphereShape(0.45f);
  btVector3 inertia(0.f, 0.f, 0.f);
  _shape->calculateLocalInertia(1.f, inertia);
  btRigidBody::btRigidBodyConstructionInfo riginfo(1.f, nullptr, _shape, inertia);

  btTransform ws;
  ws.setIdentity();
  ws.setOrigin(btVector3(position.x, position.y, position.z));
  riginfo.m_startWorldTransform = ws;
  riginfo.m_mass = 1.f;

  _body = new btRigidBody(riginfo);
  _body->setUserPointer(this);
  _body->setLinearVelocity(btVector3(_info.direction.x, _info.direction.y, _info.direction.z) *
                           _info.velocity);
  engine->dynamicsWorld->addRigidBody(
      _body, btBroadphaseProxy::DefaultFilter,
      btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter);

  if (_info.type == RPG) {
    // RPGs aren't affected by gravity
    _body->setGravity({0.f, 0.f, 0.f});
  }

  if (_info.type != Grenade) {
    // Projectiles that aren't grenades explode on contact.
    _ghostBody = new btPairCachingGhostObject();
    _ghostBody->setWorldTransform(_body->getWorldTransform());
    _ghostBody->setCollisionShape(_shape);
    _ghostBody->setUserPointer(this);
    _ghostBody->setCollisionFlags(btCollisionObject::CF_KINEMATIC_OBJECT |
                                  btCollisionObject::CF_NO_CONTACT_RESPONSE);
    engine->dynamicsWorld->addCollisionObject(_ghostBody, btBroadphaseProxy::SensorTrigger,
                                              btBroadphaseProxy::AllFilter);
  }
}

ProjectileObject::~ProjectileObject() { cleanup(); }

void ProjectileObject::tick(float dt)
{
  if (_body == nullptr) return;

  auto& bttr = _body->getWorldTransform();
  position = {bttr.getOrigin().x(), bttr.getOrigin().y(), bttr.getOrigin().z()};
  auto r = bttr.getRotation();
  rotation = {r.x(), r.y(), r.z(), r.w()};

  _info.time -= dt;

  if (_ghostBody) {
    _ghostBody->setWorldTransform(_body->getWorldTransform());
    checkPhysicsContact();
  }

  if (_info.time <= 0.f) {
    explode();
  }
}
