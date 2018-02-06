#include <boost/test/unit_test.hpp>
#include <engine/GameWorld.hpp>
#include <objects/InstanceObject.hpp>
#include "test_Globals.hpp"

BOOST_AUTO_TEST_SUITE(ObjectTests)

#if 0  // Tests disabled as object damage logic is unclear

BOOST_AUTO_TEST_CASE(instance_test_damage)
{
	std::shared_ptr<ObjectData> object(new ObjectData);
	InstanceObject inst(Global::get().e,
					 glm::vec3(0.f, 0.f, 0.f),
					 glm::quat{1.0f,0.0f,0.0f,0.0f}, nullptr,
					 glm::vec3(1.f),
					 object, nullptr, nullptr
					);
	GameObject::DamageInfo dmg;
	dmg.type = GameObject::DamageInfo::Bullet;
	dmg.hitpoints = 50.f;

	// Set object to undamagable.
	object->flags = 0;

	BOOST_CHECK( ! inst.takeDamage(dmg) );

	BOOST_CHECK( inst.takeDamage(dmg) );

	BOOST_CHECK( inst.getHealth() < 0.f );
}

BOOST_AUTO_TEST_CASE(instance_test_destroy)
{
	std::shared_ptr<ObjectData> object(new ObjectData);
	InstanceObject inst(Global::get().e,
					 glm::vec3(0.f, 0.f, 0.f),
					 glm::quat{1.0f,0.0f,0.0f,0.0f}, nullptr,
					 glm::vec3(1.f),
					 object, nullptr, nullptr
					);
	GameObject::DamageInfo dmg;
	dmg.type = GameObject::DamageInfo::Bullet;
	dmg.hitpoints = inst.getHealth() + 1.f;

	BOOST_CHECK( inst.takeDamage(dmg) );

	BOOST_CHECK( inst.getHealth() < 0.f );
}

#endif

BOOST_AUTO_TEST_SUITE_END()
