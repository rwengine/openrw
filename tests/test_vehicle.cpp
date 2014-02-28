#include <boost/test/unit_test.hpp>
#include "test_globals.hpp"
#include <objects/GTAVehicle.hpp>

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
}

BOOST_AUTO_TEST_SUITE_END()

