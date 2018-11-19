#include <boost/test/unit_test.hpp>
#include <objects/VehicleObject.hpp>
#include "test_Globals.hpp"

BOOST_AUTO_TEST_SUITE(BuoyancyTests)

#if RW_TEST_WITH_DATA
BOOST_AUTO_TEST_CASE(test_vehicle_buoyancy) {
    glm::vec2 tpos(-WATER_WORLD_SIZE / 2.f + 10.f);
    {
        VehicleObject* vehicle = Global::get().e->createVehicle(
            90u, glm::vec3(tpos, 100.f), glm::quat{1.0f,0.0f,0.0f,0.0f});

        BOOST_REQUIRE(vehicle != nullptr);

        BOOST_REQUIRE(vehicle->info != nullptr);
        BOOST_REQUIRE(vehicle->getVehicle() != nullptr);

        // Relies on tile 0,0 being watered...

        BOOST_CHECK(!vehicle->isInWater());

        // Move it under the water
        vehicle->setPosition(glm::vec3(tpos, -5.f));

        // Allow the object to update
        vehicle->tickPhysics(0.0016f);

        BOOST_CHECK(vehicle->isInWater());

        // Ensure that the in water state sticks
        vehicle->tickPhysics(0.0016f);

        BOOST_CHECK(vehicle->isInWater());

        vehicle->setPosition(glm::vec3(tpos, 5.f));
        vehicle->tickPhysics(0.0016f);
        BOOST_CHECK(!vehicle->isInWater());

        // TODO(danhedron): fix magic numbers
        auto orgval = Global::get().e->data->realWater[0];
        Global::get().e->data->realWater[0] = NO_WATER_INDEX;

        vehicle->tickPhysics(0.0016f);
        BOOST_CHECK(!vehicle->isInWater());

        vehicle->setPosition(glm::vec3(tpos, -5.f));

        vehicle->tickPhysics(0.0016f);
        BOOST_CHECK(!vehicle->isInWater());

        Global::get().e->data->realWater[0] = orgval;

        vehicle->tickPhysics(0.0016f);
        BOOST_CHECK(!vehicle->isInWater());

        Global::get().e->destroyObject(vehicle);
    }
}
#endif

BOOST_AUTO_TEST_SUITE_END()
