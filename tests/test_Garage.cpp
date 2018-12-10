#include <boost/test/unit_test.hpp>
#include <engine/Garage.hpp>
#include "test_Globals.hpp"
#if RW_TEST_WITH_DATA

BOOST_AUTO_TEST_SUITE(GarageTests)

BOOST_AUTO_TEST_CASE(test_garage_interaction) {
    {
        auto garage = Global::get().e->createGarage(
            {0.f, 0.f, 0.f}, {3.f, 3.f, 3.f}, GarageType::Respray);
        BOOST_REQUIRE(garage != nullptr);
    }
}

BOOST_AUTO_TEST_SUITE_END()

#endif
