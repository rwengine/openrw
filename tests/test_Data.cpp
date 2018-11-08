#include <boost/test/unit_test.hpp>
#include <data/WeaponData.hpp>
#include <loaders/GenericDATLoader.hpp>
#include <objects/PickupObject.hpp>
#include <objects/InstanceObject.hpp>
#include "test_Globals.hpp"

// Tests against loading various data files
// These tests are bad but so are the interfaces so it cancels out.

BOOST_AUTO_TEST_SUITE(DataTests)

#if RW_TEST_WITH_DATA
BOOST_AUTO_TEST_CASE(test_weapon_dat) {
    GenericDATLoader l;
    std::vector<WeaponData> weaponData;

    l.loadWeapons(Global::get().getGamePath() + "/data/weapon.dat", weaponData);

    BOOST_ASSERT(weaponData.size() > 0);

    const auto& data = weaponData.front();

    BOOST_CHECK(data.fireType == WeaponData::MELEE);
    BOOST_CHECK(data.hitRange == 2.8f);
    BOOST_CHECK(data.fireRate == 750);
    BOOST_CHECK(data.reloadMS == 300);
    BOOST_CHECK(data.clipSize == 1000);
    BOOST_CHECK(data.damage == 8);
    BOOST_CHECK(data.speed == -1.f);
    BOOST_CHECK(data.meleeRadius == 0.5f);
    BOOST_CHECK(data.lifeSpan == -1.0f);
    BOOST_CHECK(data.spread == -1.0f);
    BOOST_CHECK(data.fireOffset == glm::vec3(0.1f, 0.65f, 0.2f));
    BOOST_CHECK(data.animation1 == "fightppunch");
    BOOST_CHECK(data.animation2 == "kick_floor");
    BOOST_CHECK(data.animLoopStart == 0);
    BOOST_CHECK(data.animLoopEnd == 99);
    BOOST_CHECK(data.animFirePoint == 12);
    BOOST_CHECK(data.animCrouchFirePoint == 12);
    BOOST_CHECK(data.modelID == -1);
    BOOST_CHECK(data.flags == 0);
}

BOOST_AUTO_TEST_CASE(test_dynamic_dat_loader) {
    GenericDATLoader l;
    std::unordered_map<std::string, DynamicObjectData> dynamicObjects;

    l.loadDynamicObjects(Global::get().getGamePath() + "/data/object.dat",
                         dynamicObjects);

    BOOST_ASSERT(!dynamicObjects.empty());

    BOOST_ASSERT(dynamicObjects.find("wastebin") != dynamicObjects.end());
    BOOST_ASSERT(dynamicObjects.find("lamppost1") != dynamicObjects.end());

    auto lamp = dynamicObjects.at("lamppost1");

    BOOST_CHECK_EQUAL(lamp.mass, 600.0);
    BOOST_CHECK_EQUAL(lamp.turnMass, 4000.0);
    BOOST_CHECK_CLOSE(lamp.airRes, 0.99, 1.0);
    BOOST_CHECK_CLOSE(lamp.elasticity, 0.05, 0.01);
    BOOST_CHECK_EQUAL(lamp.buoyancy, 50.0);
    BOOST_CHECK_EQUAL(lamp.uprootForce, 400);
    BOOST_CHECK_EQUAL(lamp.collDamageMulti, 1.0);
    BOOST_CHECK_EQUAL(lamp.collDamageEffect, 1);
    BOOST_CHECK_EQUAL(lamp.collResponseFlags, 1);
    BOOST_CHECK_EQUAL(lamp.cameraAvoid, false);
}

BOOST_AUTO_TEST_CASE(test_handling_data_loader) {
    GenericDATLoader l;
    VehicleInfoPtrs loaded;

    l.loadHandling(Global::get().getGamePath() + "/data/handling.cfg", loaded);

    BOOST_ASSERT(loaded.size() > 0);
    BOOST_ASSERT(loaded.find("STINGER") != loaded.end());

    VehicleInfoPtr info = loaded["STINGER"];
    VehicleHandlingInfo& handling = info->handling;

    BOOST_CHECK_EQUAL(handling.mass, 1000.0);
    BOOST_CHECK_EQUAL(handling.flags, 0xA182);
    BOOST_CHECK_EQUAL(handling.driveType, VehicleHandlingInfo::All);
    BOOST_CHECK_EQUAL(handling.engineType, VehicleHandlingInfo::Petrol);
}

BOOST_AUTO_TEST_CASE(test_model_files_loaded) {
    auto& d = Global::get().d;

    // The weapon models should be associated by the MODELFILE entries
    auto ak47 = d->findModelInfo<SimpleModelInfo>(171);

    BOOST_CHECK_EQUAL(ak47->name, "ak47");
    BOOST_CHECK_NE(ak47->getAtomic(0), nullptr);
}

BOOST_AUTO_TEST_CASE(test_model_archive_loaded) {
    auto& d = Global::get().d;
    auto& e = Global::get().e;

    /// @todo Implement streaming
    // Currently, instanciating an entity will load the model
    {
        auto crim = d->findModelInfo<PedModelInfo>(24);
        auto pickup = e->createPickup({}, 24, PickupObject::InShop);

        BOOST_REQUIRE(crim->type() == ModelDataType::PedInfo);
        BOOST_CHECK_NE(crim->getModel(), nullptr);

        e->destroyObject(pickup);
    }
    {
        auto info = d->findModelInfo<SimpleModelInfo>(2202);
        auto inst = e->createInstance(2202, {});

        BOOST_REQUIRE(info->type() == ModelDataType::SimpleInfo);
        BOOST_CHECK_NE(info->getAtomic(0), nullptr);

        e->destroyObject(inst);
    }
}
#endif

BOOST_AUTO_TEST_SUITE_END()
