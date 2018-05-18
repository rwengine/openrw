#include <boost/test/unit_test.hpp>
#include <data/WeaponData.hpp>
#include <objects/CharacterObject.hpp>
#include <objects/PickupObject.hpp>
#include "test_Globals.hpp"
#if RW_TEST_WITH_DATA

class TestPickup : public PickupObject {
public:
    bool picked_up = false;

    TestPickup(GameWorld* engine, const glm::vec3& position)
        : PickupObject(engine, position, Global::get().d->modelinfo[0].get(),
                       OnStreet) {
    }

    bool onCharacterTouch(CharacterObject*) {
        picked_up = true;
        return true;
    }
};

BOOST_AUTO_TEST_SUITE(PickupTests)

BOOST_AUTO_TEST_CASE(test_pickup_interaction) {
    {
        auto character =
            Global::get().e->createPedestrian(1, {30.1f, 0.f, 0.f});
        BOOST_REQUIRE(character != nullptr);

        TestPickup* p = new TestPickup(Global::get().e, {30.f, 0.f, 0.f});

        // Global::get().e->insertObject(p);

        BOOST_CHECK(!p->picked_up);

        Global::get().e->dynamicsWorld->stepSimulation(0.016f);
        p->tick(0.f);

        BOOST_CHECK(p->picked_up);

        p->picked_up = false;

        BOOST_CHECK(!p->picked_up);

        Global::get().e->dynamicsWorld->stepSimulation(0.016f);
        p->tick(0.f);

        BOOST_CHECK(!p->picked_up);

        Global::get().e->dynamicsWorld->stepSimulation(0.016f);
        p->tick(60.5f);

        BOOST_CHECK(p->picked_up);

        Global::get().e->destroyObject(p);
        Global::get().e->destroyObject(character);
    }
}

BOOST_AUTO_TEST_CASE(test_item_pickup) {
    {
        auto character =
            Global::get().e->createPedestrian(1, {30.1f, 0.f, 0.f});
        BOOST_REQUIRE(character != nullptr);

        auto pistol = Global::get().d->weaponData[1].get();
        auto model = Global::get().d->modelinfo[pistol->modelID].get();

        ItemPickup* p = new ItemPickup(Global::get().e, {30.f, 0.f, 0.f}, model,
                                       PickupObject::OnStreet, pistol);

        Global::get().e->allObjects.push_back(p);

        // Check the characters inventory is empty.
        for (int i = 0; i < kMaxInventorySlots; ++i) {
            BOOST_CHECK(character->getCurrentState().weapons[i].weaponId == 0);
        }

        Global::get().e->dynamicsWorld->stepSimulation(0.016f);
        p->tick(0.f);

        auto& inventory = character->getCurrentState().weapons;
        BOOST_CHECK(std::any_of(std::begin(inventory), std::end(inventory),
                                [&](const CharacterWeaponSlot& i) {
                                    return i.weaponId == pistol->inventorySlot;
                                }));

        Global::get().e->destroyObject(p);
        Global::get().e->destroyObject(character);
    }
}

BOOST_AUTO_TEST_SUITE_END()

#endif
