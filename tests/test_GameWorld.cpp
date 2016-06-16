#include <boost/test/unit_test.hpp>
#include <engine/GameWorld.hpp>
#include <engine/GameData.hpp>
#include <objects/InstanceObject.hpp>
#include <test_globals.hpp>

BOOST_AUTO_TEST_SUITE(GameWorldTests)

#if RW_TEST_WITH_DATA
BOOST_AUTO_TEST_CASE(test_gameobject_id)
{
	GameWorld gw(&Global::get().log, &Global::get().work, Global::get().d);
	
	auto object1 = gw.createInstance(1337, glm::vec3(100.f, 0.f, 0.f));
	auto object2 = gw.createInstance(1337, glm::vec3(100.f, 0.f, 100.f));

	BOOST_CHECK_NE( object1->getGameObjectID(), object2->getGameObjectID() );
}
#endif

BOOST_AUTO_TEST_SUITE_END()
