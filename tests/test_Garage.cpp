#include <boost/test/unit_test.hpp>
#include <engine/Garage.hpp>
#include "test_Globals.hpp"

BOOST_AUTO_TEST_SUITE(GarageTests)

BOOST_AUTO_TEST_CASE(test_garage_interaction,
                     * utf::precondition(with_data{})) {
    {
        auto garage = Global::get().e->createGarage(
            {0.f, 0.f, 0.f}, {3.f, 3.f, 3.f}, Garage::Type::Respray);
        BOOST_REQUIRE(garage != nullptr);
    }
}

BOOST_AUTO_TEST_SUITE_END()

