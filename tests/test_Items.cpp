#include <boost/test/unit_test.hpp>
#include <objects/CharacterObject.hpp>
#include "test_Globals.hpp"

BOOST_AUTO_TEST_SUITE(ItemsTests, DATA_TEST_PREDICATE)

BOOST_AUTO_TEST_CASE(test_character_inventory) {
    {
        auto character = Global::get().e->createPedestrian(1, {0.f, 0.f, 0.f});
        BOOST_REQUIRE(character != nullptr);

        character->addToInventory(1, 10);

        BOOST_CHECK_EQUAL(character->getActiveItem(), 0);

        character->setActiveItem(1);

        BOOST_CHECK_EQUAL(character->getActiveItem(), 1);

        character->removeFromInventory(1);

        BOOST_CHECK_EQUAL(character->getActiveItem(), 0);

        Global::get().e->destroyObject(character);
    }
}

BOOST_AUTO_TEST_SUITE_END()
