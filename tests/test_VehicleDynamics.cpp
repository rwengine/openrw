#include <boost/test/unit_test.hpp>
#include "test_globals.hpp"
#include <dynamics/VehicleDynamics.hpp>
#include <objects/VehicleObject.hpp>

BOOST_AUTO_TEST_SUITE(VehicleDynamicsTests)

BOOST_AUTO_TEST_CASE(test_wheels)
{
	VehicleDynamics dynamics(nullptr);
	const glm::vec3 axle = glm::vec3(1.f, 0.f, 0.f);
	const float radius = 0.25f;

	dynamics.addWheel(glm::vec3( 1.f, 1.f, 0.f), axle, radius);
	dynamics.addWheel(glm::vec3(-1.f, 1.f, 0.f), axle, radius);
	dynamics.addWheel(glm::vec3( 1.f,-1.f, 0.f), axle, radius);
	dynamics.addWheel(glm::vec3(-1.f,-1.f, 0.f), axle, radius);

	BOOST_CHECK_EQUAL(4, dynamics.getWheels().size());

	auto& wheels = dynamics.getWheels();
	BOOST_CHECK_EQUAL(1.f, wheels[0].position.x);
	BOOST_CHECK_EQUAL(1.f, wheels[0].position.y);

	BOOST_CHECK_EQUAL(-1.f, wheels[3].position.x);
	BOOST_CHECK_EQUAL(-1.f, wheels[3].position.y);
}

BOOST_AUTO_TEST_CASE(test_dynamics)
{
	VehicleObject* vehicle = Global::get().e->createVehicle(90u, glm::vec3(), glm::quat());

	BOOST_REQUIRE(vehicle != nullptr);

	BOOST_REQUIRE(vehicle->info != nullptr);
	BOOST_REQUIRE(vehicle->vehicle != nullptr);

	// We should have the vehicle rest on a surface
	// but I can't be bothered to write that now.
	// gg me rip

	Global::get().e->destroyObject(vehicle);
}

BOOST_AUTO_TEST_SUITE_END()
