#include <boost/test/unit_test.hpp>
#include "test_globals.hpp"
#include <objects/InstanceObject.hpp>
#include <engine/GameWorld.hpp>

BOOST_AUTO_TEST_SUITE(ObjectUnitTests)

BOOST_AUTO_TEST_CASE(instance_test_damage)
{
	std::shared_ptr<ObjectData> object(new ObjectData);
	InstanceObject inst(Global::get().e, 
					 glm::vec3(0.f, 0.f, 0.f), 
					 glm::quat(), nullptr, 
					 glm::vec3(1.f),
					 object, nullptr, nullptr
					);
	GameObject::DamageInfo dmg;
	dmg.type = GameObject::DamageInfo::Bullet;
	dmg.hitpoints = 50.f;
	
	// Set object to undamagable.
	object->flags = 0;
	
	BOOST_CHECK( ! inst.takeDamage(dmg) );
	
	// Now make it explode on hit
	
	object->flags = ObjectData::EXPLODEONHIT;
	
	BOOST_CHECK( inst.takeDamage(dmg) );
	
	BOOST_CHECK( inst.getHealth() < 0.f );
}

BOOST_AUTO_TEST_CASE(instance_test_destroy)
{
	std::shared_ptr<ObjectData> object(new ObjectData);
	InstanceObject inst(Global::get().e,
					 glm::vec3(0.f, 0.f, 0.f), 
					 glm::quat(), nullptr, 
					 glm::vec3(1.f),
					 object, nullptr, nullptr
					);
	GameObject::DamageInfo dmg;
	dmg.type = GameObject::DamageInfo::Bullet;
	dmg.hitpoints = inst.getHealth() + 1.f;
	
	// Now make it damageable
	
	object->flags = ObjectData::EXPLODEONHIT;
	
	BOOST_CHECK( inst.takeDamage(dmg) );
	
	BOOST_CHECK( inst.getHealth() < 0.f );
}

BOOST_AUTO_TEST_SUITE_END()
