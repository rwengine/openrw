#include <boost/test/unit_test.hpp>
#include <engine/GameWorld.hpp>
#include <engine/GameData.hpp>
#include <test_globals.hpp>

BOOST_AUTO_TEST_SUITE(GameWorldTests)

BOOST_AUTO_TEST_CASE(test_gameobject_id)
{
	GameData gd(&Global::get().log, &Global::get().work, Global::getGamePath());
	GameWorld gw(&Global::get().log, &Global::get().work, &gd);
	
	auto object1 = gw.createInstance(1337, glm::vec3(100.f, 0.f, 0.f));
	auto object2 = gw.createInstance(1337, glm::vec3(100.f, 0.f, 0.f));

	BOOST_CHECK_NE( object1->getObjectID(), object2->getObjectID() );
}

BOOST_AUTO_TEST_SUITE_END()