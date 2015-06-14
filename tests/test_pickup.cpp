#include <boost/test/unit_test.hpp>
#include <objects/PickupObject.hpp>
#include <objects/ItemPickup.hpp>
#include <items/InventoryItem.hpp>
#include <data/WeaponData.hpp>
#include <objects/CharacterObject.hpp>
#include "test_globals.hpp"

class TestPickup : public PickupObject
{
public:
	bool picked_up = false;

	TestPickup(GameWorld* engine, const glm::vec3& position)
		: PickupObject(engine, position, 0)
	{}

	bool onCharacterTouch(CharacterObject *character) {
		picked_up = true;
		return true;
	}
};

BOOST_AUTO_TEST_SUITE(PickupTests)

BOOST_AUTO_TEST_CASE(test_pickup_interaction)
{
	{
		auto character = Global::get().e->createPedestrian(1, { 30.1f, 0.f, 0.f });
		BOOST_REQUIRE( character != nullptr );

		TestPickup* p = new TestPickup(Global::get().e, { 30.f, 0.f, 0.f } );

		Global::get().e->insertObject(p);

		BOOST_CHECK( ! p->picked_up );

		Global::get().e->dynamicsWorld->stepSimulation(0.016f);
		p->tick(0.f);

		BOOST_CHECK( p->picked_up );

		p->picked_up = false;

		BOOST_CHECK( ! p->picked_up );

		Global::get().e->dynamicsWorld->stepSimulation(0.016f);
		p->tick(0.f);

		BOOST_CHECK( ! p->picked_up );

		Global::get().e->dynamicsWorld->stepSimulation(0.016f);
		p->tick(60.5f);

		BOOST_CHECK( p->picked_up );


		Global::get().e->destroyObject(p);
		Global::get().e->destroyObject(character);
	}
}

BOOST_AUTO_TEST_CASE(test_item_pickup)
{
	{
		auto character = Global::get().e->createPedestrian(1, { 30.1f, 0.f, 0.f });
		BOOST_REQUIRE( character != nullptr );

		auto item = Global::get().e->data->weaponData[9];

		ItemPickup* p = new ItemPickup(Global::get().e, { 30.f, 0.f, 0.f }, item );

		Global::get().e->insertObject(p);

		// Check the characters inventory is empty.
		BOOST_CHECK( character->getInventory().empty() );

		Global::get().e->dynamicsWorld->stepSimulation(0.016f);
		p->tick(0.f);

		BOOST_CHECK( ! character->getInventory().empty() );

		auto inventory = character->getInventory();
		BOOST_CHECK( std::any_of( inventory.begin(), inventory.end(),
								  [&](const std::pair<int, InventoryItem*>& i)
		{ return i.second->getModelID() == item->modelID; }) );

		Global::get().e->destroyObject(p);
		Global::get().e->destroyObject(character);
	}
}

BOOST_AUTO_TEST_SUITE_END()

