#include <boost/test/unit_test.hpp>
#include <data/WeaponData.hpp>
#include "test_globals.hpp"
#include <loaders/GenericDATLoader.hpp>

// Tests against loading various data files
// These tests are bad but so are the interfaces so it cancels out.

BOOST_AUTO_TEST_SUITE(DataTests)

#if RW_TEST_WITH_DATA
BOOST_AUTO_TEST_CASE(test_weapon_dat)
{
	GenericDATLoader l;
	WeaponDataPtrs weaponData;

	l.loadWeapons(Global::get().getGamePath() + "/data/weapon.dat", weaponData);

	BOOST_ASSERT( weaponData.size() > 0 );

	WeaponDataPtr data = weaponData[0];

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

BOOST_AUTO_TEST_CASE(test_dynamic_dat_loader)
{
	GenericDATLoader l;
	DynamicObjectDataPtrs loaded;

	l.loadDynamicObjects(Global::get().getGamePath() + "/data/object.dat", loaded);

	BOOST_ASSERT( loaded.size() > 0 );

	BOOST_ASSERT( loaded.find("wastebin") != loaded.end() );
	BOOST_ASSERT( loaded.find("lamppost1") != loaded.end() );

	DynamicObjectDataPtr lamp = loaded["lamppost1"];

	BOOST_CHECK_EQUAL( lamp->mass, 600.0);
	BOOST_CHECK_EQUAL( lamp->turnMass, 4000.0);
	BOOST_CHECK_CLOSE( lamp->airRes, 0.99, 1.0);
	BOOST_CHECK_CLOSE( lamp->elacticity, 0.05, 0.01);
	BOOST_CHECK_EQUAL( lamp->bouancy, 50.0);
	BOOST_CHECK_EQUAL( lamp->uprootForce, 400);
	BOOST_CHECK_EQUAL( lamp->collDamageMulti, 1.0);
	BOOST_CHECK_EQUAL( lamp->collDamageFlags, 1);
	BOOST_CHECK_EQUAL( lamp->collResponseFlags, 1);
	BOOST_CHECK_EQUAL( lamp->cameraAvoid, false);
}

BOOST_AUTO_TEST_CASE(test_handling_data_loader)
{
	GenericDATLoader l;
	VehicleInfoPtrs loaded;

	l.loadHandling(Global::get().getGamePath() + "/data/handling.cfg", loaded);

	BOOST_ASSERT( loaded.size() > 0 );
	BOOST_ASSERT( loaded.find("STINGER") != loaded.end() );

	VehicleInfoPtr info = loaded["STINGER"];
	VehicleHandlingInfo& handling = info->handling;

	BOOST_CHECK_EQUAL( handling.mass, 1000.0 );
	BOOST_CHECK_EQUAL( handling.flags, 0xA182 );
	BOOST_CHECK_EQUAL( handling.driveType, VehicleHandlingInfo::All );
	BOOST_CHECK_EQUAL( handling.engineType, VehicleHandlingInfo::Petrol );
}
#endif

BOOST_AUTO_TEST_SUITE_END()

