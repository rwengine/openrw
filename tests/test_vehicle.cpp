#include <boost/test/unit_test.hpp>
#include "test_globals.hpp"
#include <objects/GTAVehicle.hpp>
#include <render/Model.hpp>

BOOST_AUTO_TEST_SUITE(VehicleTests)

BOOST_AUTO_TEST_CASE(test_create_vehicle)
{
	GTAVehicle* vehicle = Global::get().e->createVehicle(90u, glm::vec3(), glm::quat());

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
	GTAVehicle* vehicle = Global::get().e->createVehicle(90u, glm::vec3(), glm::quat());

	BOOST_REQUIRE(vehicle != nullptr);
	BOOST_REQUIRE(vehicle->model != nullptr);

	auto bonnet_ok = vehicle->model->model->findFrame("bonnet_hi_ok");
	auto bonnet_dam = vehicle->model->model->findFrame("bonnet_hi_dam");

	BOOST_REQUIRE(bonnet_ok != nullptr);
	BOOST_REQUIRE(bonnet_dam != nullptr);
	BOOST_CHECK(vehicle->isFrameVisible(bonnet_ok));
	BOOST_CHECK(!vehicle->isFrameVisible(bonnet_dam));

	vehicle->setPartDamaged(GTAVehicle::DF_Bonnet, true);

	BOOST_CHECK(!vehicle->isFrameVisible(bonnet_ok));
	BOOST_CHECK(vehicle->isFrameVisible(bonnet_dam));

	vehicle->setPartDamaged(GTAVehicle::DF_Bonnet, false);

	BOOST_CHECK(vehicle->isFrameVisible(bonnet_ok));
	BOOST_CHECK(!vehicle->isFrameVisible(bonnet_dam));

	Global::get().e->destroyObject(vehicle);
}

BOOST_AUTO_TEST_CASE(test_door_position)
{
	GTAVehicle* vehicle = Global::get().e->createVehicle(90u, glm::vec3(10.f, 0.f, 0.f), glm::quat());

	BOOST_REQUIRE(vehicle != nullptr);

	BOOST_REQUIRE(vehicle->info != nullptr);
	BOOST_REQUIRE(vehicle->vehicle != nullptr);

	BOOST_CHECK( vehicle->getSeatEntryPosition(0).x > 5.f );


	Global::get().e->destroyObject(vehicle);
}
BOOST_AUTO_TEST_SUITE_END()

