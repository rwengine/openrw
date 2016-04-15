#include <boost/test/unit_test.hpp>
#include <items/WeaponItem.hpp>
#include <objects/CharacterObject.hpp>
#include "test_globals.hpp"

BOOST_AUTO_TEST_SUITE(ItemTests)
BOOST_AUTO_TEST_CASE(test_character_inventory)
{
	{
		auto character = Global::get().e->createPedestrian(1, {0.f, 0.f, 0.f});
		BOOST_REQUIRE( character != nullptr );

		auto item = Global::get().e->getInventoryItem(4);

		BOOST_REQUIRE(item != nullptr);

		character->addToInventory(item);

		character->setActiveItem( item->getInventorySlot() );

		BOOST_CHECK_EQUAL( character->getActiveItem(), item );

		character->removeFromInventory( item->getInventorySlot() );

		BOOST_CHECK_EQUAL( character->getActiveItem(), nullptr );

		Global::get().e->destroyObject(character);
	}
}

BOOST_AUTO_TEST_SUITE_END()

