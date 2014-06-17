#include <boost/test/unit_test.hpp>
#include "test_globals.hpp"
#include <objects/VehicleObject.hpp>
#include <render/Model.hpp>

BOOST_AUTO_TEST_SUITE(VehicleTests)

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

BOOST_AUTO_TEST_CASE(vehicle_frame_flags)
{
	VehicleObject* vehicle = Global::get().e->createVehicle(90u, glm::vec3(), glm::quat());

	BOOST_REQUIRE(vehicle != nullptr);
	BOOST_REQUIRE(vehicle->model != nullptr);

	auto bonnet_ok = vehicle->model->model->findFrame("bonnet_hi_ok");
	auto bonnet_dam = vehicle->model->model->findFrame("bonnet_hi_dam");

	BOOST_REQUIRE(bonnet_ok != nullptr);
	BOOST_REQUIRE(bonnet_dam != nullptr);
	BOOST_CHECK(vehicle->isFrameVisible(bonnet_ok, 0.f));
	BOOST_CHECK(!vehicle->isFrameVisible(bonnet_dam, 0.f));

	vehicle->setPartDamaged(VehicleObject::DF_Bonnet, true);

	BOOST_CHECK(!vehicle->isFrameVisible(bonnet_ok, 0.f));
	BOOST_CHECK(vehicle->isFrameVisible(bonnet_dam, 0.f));

	vehicle->setPartDamaged(VehicleObject::DF_Bonnet, false);

	BOOST_CHECK(vehicle->isFrameVisible(bonnet_ok, 0.f));
	BOOST_CHECK(!vehicle->isFrameVisible(bonnet_dam, 0.f));

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
BOOST_AUTO_TEST_SUITE_END()

