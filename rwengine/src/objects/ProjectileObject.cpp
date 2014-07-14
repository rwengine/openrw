#include <objects/ProjectileObject.hpp>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <engine/GameWorld.hpp>
#include <data/WeaponData.hpp>

void ProjectileObject::explode()
{
	/// @todo accelerate this with bullet instead of doing this stupid loop.
	for(auto& o : engine->objects) {
		if( o == this ) continue;
		switch( o->type() ) {
		case GameObject::Instance:
		case GameObject::Vehicle:
		case GameObject::Character:
			break;
		default:
			continue;
		}

		float d = glm::distance(getPosition(), o->getPosition());
		if( d > 10.f ) continue;

		o->takeDamage({
						  getPosition(),
						  getPosition(),
						  10.f / glm::max(d, 1.f),
						  DamageInfo::Explosion,
						  0.f
					  });
	}
}

ProjectileObject::ProjectileObject(GameWorld *world, const glm::vec3 &position, const ProjectileObject::ProjectileInfo &info)
	: GameObject(world, position, glm::quat(), nullptr), _info(info),
	  _body(nullptr)
{
	_shape = new btSphereShape(0.25f);
	btVector3 inertia(0.f, 0.f, 0.f);
	_shape->calculateLocalInertia(1.f, inertia);
	btRigidBody::btRigidBodyConstructionInfo riginfo(1.f, nullptr, _shape, inertia);

	riginfo.m_startWorldTransform = btTransform(btQuaternion(), btVector3(position.x, position.y, position.z));
	riginfo.m_mass = 1.f;

	_body = new btRigidBody(riginfo);
	_body->setUserPointer(this);
	_body->setLinearVelocity(btVector3(_info.direction.x, _info.direction.y, _info.direction.z) * _info.velocity);
	engine->dynamicsWorld->addRigidBody(_body);

	if( _info.type == RPG ) {
		// RPGs aren't affected by gravity
		_body->setGravity( { 0.f, 0.f, 0.f } );
	}
}

ProjectileObject::~ProjectileObject()
{
	if( _body ) {
		engine->dynamicsWorld->removeCollisionObject(_body);
		delete _body;
	}
	if( _shape ) {
		delete _shape;
	}
}

void ProjectileObject::tick(float dt)
{
	auto& bttr = _body->getWorldTransform();
	position = { bttr.getOrigin().x(), bttr.getOrigin().y(), bttr.getOrigin().z() };

	_info.time -= dt;

	/// @todo implement contact callbacks or GhostObject for detecting overlaps for molotovs and RPGs.

	if( _info.time <= 0.f ) {
		explode();
		/// @todo request the world delete us
	}
}
