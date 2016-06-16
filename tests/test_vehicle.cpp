#include <boost/test/unit_test.hpp>
#include "test_globals.hpp"
#include <objects/VehicleObject.hpp>
#include <data/Model.hpp>
#include <data/Skeleton.hpp>

BOOST_AUTO_TEST_SUITE(VehicleTests)

#if RW_TEST_WITH_DATA
BOOST_AUTO_TEST_CASE(test_create_vehicle)
{
	VehicleObject* vehicle = Global::get().e->createVehicle(90u, glm::vec3(), glm::quat());

	BOOST_REQUIRE(vehicle != nullptr);

	BOOST_REQUIRE(vehicle->info != nullptr);
	BOOST_REQUIRE(vehicle->vehicle != nullptr);

	// Hardcoded values for the moment
	BOOST_CHECK_EQUAL(vehicle->vehicle->type, VehicleData::CAR);

	BOOST_CHECK_EQUAL(vehicle->info->wheels.size(), 4);

	BOOST_CHECK_EQUAL(vehicle->info->seats.size(), 4);

	Global::get().e->destroyObject(vehicle);
}

BOOST_AUTO_TEST_CASE(vehicle_parts)
{
	VehicleObject* vehicle = Global::get().e->createVehicle(90u, glm::vec3(), glm::quat());

	BOOST_REQUIRE(vehicle != nullptr);
	BOOST_REQUIRE(vehicle->model != nullptr);
	
	VehicleObject::Part* part = vehicle->getPart("bonnet_dummy");
	
	BOOST_REQUIRE( part != nullptr );
	
	BOOST_REQUIRE( part->normal != nullptr );
	BOOST_REQUIRE( part->damaged != nullptr );
	
	BOOST_CHECK_EQUAL( part->normal->getName(), "bonnet_hi_ok");
	BOOST_CHECK_EQUAL( part->damaged->getName(), "bonnet_hi_dam");
	
	Global::get().e->destroyObject(vehicle);
}

BOOST_AUTO_TEST_CASE(vehicle_part_vis)
{
	VehicleObject* vehicle = Global::get().e->createVehicle(90u, glm::vec3(), glm::quat());

	BOOST_REQUIRE(vehicle != nullptr);
	BOOST_REQUIRE(vehicle->model != nullptr);
	
	VehicleObject::Part* bonnetpart = vehicle->getPart("bonnet_dummy");
	auto skel = vehicle->skeleton;
	
	vehicle->setPartState(bonnetpart, VehicleObject::DAM);
	
	BOOST_CHECK(!skel->getData(bonnetpart->normal->getIndex()).enabled );
	BOOST_CHECK( skel->getData(bonnetpart->damaged->getIndex()).enabled );
	
	vehicle->setPartState(bonnetpart, VehicleObject::OK);
	
	BOOST_CHECK( skel->getData(bonnetpart->normal->getIndex()).enabled );
	BOOST_CHECK(!skel->getData(bonnetpart->damaged->getIndex()).enabled );

	Global::get().e->destroyObject(vehicle);
}

BOOST_AUTO_TEST_CASE(test_door_position)
{
	VehicleObject* vehicle = Global::get().e->createVehicle(90u, glm::vec3(10.f, 0.f, 0.f), glm::quat());

	BOOST_REQUIRE(vehicle != nullptr);

	BOOST_REQUIRE(vehicle->info != nullptr);
	BOOST_REQUIRE(vehicle->vehicle != nullptr);

	BOOST_CHECK( vehicle->getSeatEntryPosition(0).x > 5.f );


	Global::get().e->destroyObject(vehicle);
}

BOOST_AUTO_TEST_CASE(test_hinges)
{
	VehicleObject* vehicle = Global::get().e->createVehicle(90u, glm::vec3(10.f, 0.f, 0.f), glm::quat());
	
	BOOST_REQUIRE( vehicle != nullptr );

	VehicleObject::Part* part = vehicle->getPart("door_lf_dummy");
	
	BOOST_REQUIRE( part != nullptr );
	
	BOOST_CHECK_EQUAL( part->constraint, nullptr );
	BOOST_CHECK_EQUAL( part->body, nullptr );
	
	vehicle->setPartLocked(part, false);
	
	BOOST_CHECK_NE( part->body, nullptr );
	BOOST_CHECK_NE( part->constraint, nullptr );
	
	vehicle->setPartLocked(part, true);

	BOOST_CHECK_EQUAL( part->constraint, nullptr );
	BOOST_CHECK_EQUAL( part->body, nullptr );
	
	Global::get().e->destroyObject(vehicle);
}

BOOST_AUTO_TEST_CASE(test_open_part)
{
	VehicleObject* vehicle = Global::get().e->createVehicle(90u, glm::vec3(10.f, 0.f, 0.f), glm::quat());
	
	BOOST_REQUIRE( vehicle != nullptr );

	VehicleObject::Part* part = vehicle->getPart("door_lf_dummy");
	
	BOOST_REQUIRE( part != nullptr );
	
	BOOST_CHECK_EQUAL( part->body, nullptr );
	
	vehicle->setPartLocked(part, true);
	vehicle->setPartTarget(part, true, 1.f);
	
	/// @todo a reasonable test
	
	Global::get().e->destroyObject(vehicle);
}
#endif

BOOST_AUTO_TEST_SUITE_END()

