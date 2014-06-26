#include <boost/test/unit_test.hpp>
#include <data/WeaponData.hpp>
#include "test_globals.hpp"

// Tests against loading various data files
// These tests are bad but so are the interfaces so it cancels out.

BOOST_AUTO_TEST_SUITE(DataTests)

BOOST_AUTO_TEST_CASE(test_weapon_dat)
{
	{
		// Verify data against some known values.
		std::shared_ptr<WeaponData> data = Global::get().e->gameData.weaponData["unarmed"];

		BOOST_ASSERT( data );

		BOOST_CHECK( data->fireType == WeaponData::MELEE );
		BOOST_CHECK( data->hitRange == 2.8f );
		BOOST_CHECK( data->fireRate == 750 );
		BOOST_CHECK( data->reloadMS == 300 );
		BOOST_CHECK( data->clipSize == 1000 );
		BOOST_CHECK( data->damage == 8);
		BOOST_CHECK( data->speed == -1.f);
		BOOST_CHECK( data->meleeRadius == 0.5f );
		BOOST_CHECK( data->lifeSpan == -1.0f );
		BOOST_CHECK( data->spread == -1.0f );
		BOOST_CHECK( data->fireOffset == glm::vec3(0.1f, 0.65f, 0.2f) );
		BOOST_CHECK( data->animation1 == "fightppunch" );
		BOOST_CHECK( data->animation2 == "kick_floor");
		BOOST_CHECK( data->animLoopStart == 0 );
		BOOST_CHECK( data->animLoopEnd == 99 );
		BOOST_CHECK( data->animFirePoint == 12 );
		BOOST_CHECK( data->animCrouchFirePoint == 12 );
		BOOST_CHECK( data->modelID == -1 );
		BOOST_CHECK( data->flags == 0) ;

	}
}

BOOST_AUTO_TEST_SUITE_END()

