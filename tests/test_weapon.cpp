#include <boost/test/unit_test.hpp>
#include "test_globals.hpp"
#include <objects/CharacterObject.hpp>
#include <data/WeaponData.hpp>

BOOST_AUTO_TEST_SUITE(WeaponTests)

BOOST_AUTO_TEST_CASE(TestWeaponScan)
{
	{
		// Test RADIUS scan
		auto character = Global::get().e->createPedestrian(1, {0.f, 0.f, 0.f});
		BOOST_REQUIRE( character != nullptr );
		BOOST_REQUIRE( character->model != nullptr);
		BOOST_REQUIRE( character->physObject != nullptr);

		WeaponScan scan( 10.f, {0.f, 0.f, 10.f}, {0.f,0.f, -10.f} );

		Global::get().e->doWeaponScan( scan );

		BOOST_CHECK( character->mHealth < 100.f );

		Global::get().e->destroyObject(character);
	}
}

BOOST_AUTO_TEST_SUITE_END()
