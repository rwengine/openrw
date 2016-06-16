#include <boost/test/unit_test.hpp>
#include <items/WeaponItem.hpp>
#include <objects/CharacterObject.hpp>
#include "test_globals.hpp"

BOOST_AUTO_TEST_SUITE(ItemTests)

#if RW_TEST_WITH_DATA
BOOST_AUTO_TEST_CASE(test_character_inventory)
{
	{
		auto character = Global::get().e->createPedestrian(1, {0.f, 0.f, 0.f});
		BOOST_REQUIRE( character != nullptr );

		auto item = Global::get().e->getInventoryItem(4);
		auto fist = Global::get().e->getInventoryItem(0);

		BOOST_REQUIRE(item != nullptr);
		BOOST_REQUIRE(fist != nullptr );
		BOOST_CHECK_NE( fist, item );

		character->addToInventory(item);

		BOOST_CHECK_EQUAL( character->getActiveItem(), fist );

		character->setActiveItem( item->getInventorySlot() );

		BOOST_CHECK_EQUAL( character->getActiveItem(), item );

		character->removeFromInventory( item->getInventorySlot() );

		BOOST_CHECK_EQUAL( character->getActiveItem(), fist );

		Global::get().e->destroyObject(character);
	}
}
#endif

BOOST_AUTO_TEST_SUITE_END()

