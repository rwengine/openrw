#include <boost/test/unit_test.hpp>
#include <glm/gtx/string_cast.hpp>
#include <objects/InstanceObject.hpp>
#include "test_globals.hpp"

BOOST_AUTO_TEST_SUITE(LifetimeTests)

BOOST_AUTO_TEST_CASE(test_cleanup)
{
	GameObject* f = Global::get().e->createInstance(1337, glm::vec3(0.f, 0.f, 1000.f));
	
	f->setLifetime(GameObject::TrafficLifetime);
	
	{
		auto search = Global::get().e->objects.find( f );
		BOOST_CHECK( search != Global::get().e->objects.end() );
	}
	
	Global::get().e->cleanupTraffic(glm::vec3(0.f, 0.f, 0.f));
	
	{
		auto search = Global::get().e->objects.find( f );
		BOOST_CHECK( search == Global::get().e->objects.end() );
	}
}

BOOST_AUTO_TEST_SUITE_END()