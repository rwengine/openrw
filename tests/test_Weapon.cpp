#include <boost/test/unit_test.hpp>
#include <data/WeaponData.hpp>
#include <items/Weapon.hpp>
#include <objects/CharacterObject.hpp>
#include <objects/ProjectileObject.hpp>
#include "test_Globals.hpp"

auto& operator<<(std::ostream& s, const ScanType& type) {
    return s << static_cast<int>(type);
}

BOOST_AUTO_TEST_SUITE(WeaponTests)

BOOST_AUTO_TEST_CASE(radius_ctor_creates_radius_scan) {
    WeaponScan scan{10.f, {1.f, 1.f, 1.f}, 5.f};
    BOOST_CHECK_EQUAL(scan.type, ScanType::Radius);
}

BOOST_AUTO_TEST_CASE(hitscan_ctor_creates_radius_scan) {
    WeaponScan scan{10.f, {1.f, 1.f, 1.f}, {0.f, 0.f, 0.f}};
    BOOST_CHECK_EQUAL(scan.type, ScanType::HitScan);
}

struct WeaponScanFixture {
    GameObject* source = reinterpret_cast<GameObject*>(0x0000BEEF);
    WeaponScan scan{10.f, {1.f, 1.f, 1.f}, {0.f, 0.f, 0.f}, nullptr, source};
};

BOOST_FIXTURE_TEST_CASE(weapon_scan_doesnt_injur_source, WeaponScanFixture) {
    BOOST_CHECK(!scan.doesDamage(reinterpret_cast<GameObject*>(0x0000BEEF)));
}

BOOST_FIXTURE_TEST_CASE(weapon_scan_does_injur_others, WeaponScanFixture) {
    BOOST_CHECK(scan.doesDamage(reinterpret_cast<GameObject*>(0xDEADBEEF)));
}

#if RW_TEST_WITH_DATA
BOOST_AUTO_TEST_CASE(TestDoWeaponScan) {
    {
        // Test RADIUS scan
        auto character = Global::get().e->createPedestrian(1, {0.f, 0.f, 0.f});
        BOOST_REQUIRE(character != nullptr);
        BOOST_REQUIRE(character->getModel() != nullptr);
        BOOST_REQUIRE(character->physObject != nullptr);

        WeaponScan scan(10.f, {0.f, 0.f, 10.f}, {0.f, 0.f, -10.f});

        Global::get().e->doWeaponScan(scan);

        BOOST_CHECK(character->getCurrentState().health < 100.f);

        Global::get().e->destroyObject(character);
    }
}

BOOST_AUTO_TEST_CASE(TestProjectile) {
    {
        auto character = Global::get().e->createPedestrian(1, {25.f, 0.f, 0.f});
        BOOST_REQUIRE(character != nullptr);

        auto wepdata = &Global::get().e->data->weaponData.at(5);

        auto projectile = new ProjectileObject(
            Global::get().e, {26.f, 1.f, 10.f},
            {ProjectileObject::Grenade, {0.f, 0.f, -1.f}, 2.0f, 5.0f, wepdata});

        Global::get().e->allObjects.push_back(projectile);

        BOOST_CHECK(character->getCurrentState().health == 100.f);

        for (float t = 0.f; t <= 5.f; t += 0.016f) {
            Global::get().e->dynamicsWorld->stepSimulation(0.016f, 0, 0);
            projectile->tick(0.016f);
        }

        BOOST_CHECK_LT(
            glm::distance(character->getPosition(), projectile->getPosition()),
            10.f);
        BOOST_CHECK_LT(
            glm::distance(character->getPosition(), projectile->getPosition()),
            5.f);

        // Grenade should have dentonated by this point
        BOOST_CHECK(character->getCurrentState().health < 100.f);

        Global::get().e->destroyObjectQueued(character);
        Global::get().e->destroyQueuedObjects();
    }

    {
        auto character = Global::get().e->createPedestrian(1, {25.f, 0.f, 0.f});
        BOOST_REQUIRE(character != nullptr);

        auto wepdata = &Global::get().e->data->weaponData.at(6);

        auto projectile = new ProjectileObject(
            Global::get().e, {26.f, 1.f, 10.f},
            {ProjectileObject::Molotov, {0.f, 0.f, -1.f}, 2.0f, 10.f, wepdata});

        Global::get().e->allObjects.push_back(projectile);

        BOOST_CHECK(character->getCurrentState().health == 100.f);

        for (float t = 0.f; t <= 9.0f; t += 0.016f) {
            Global::get().e->dynamicsWorld->stepSimulation(0.016f, 0, 0);
            projectile->tick(0.016f);
        }

        BOOST_CHECK(projectile->getPosition().z < 10.f);
        BOOST_CHECK(projectile->getPosition().z > 0.f);

        BOOST_CHECK(character->getCurrentState().health < 100.f);

        Global::get().e->destroyObjectQueued(character);
        Global::get().e->destroyQueuedObjects();
    }
    {
        auto character = Global::get().e->createPedestrian(1, {25.f, 0.f, 0.f});
        BOOST_REQUIRE(character != nullptr);

        auto wepdata = &Global::get().e->data->weaponData.at(7);

        auto projectile = new ProjectileObject(
            Global::get().e, {26.f, 1.f, 10.f},
            {ProjectileObject::RPG, {0.f, 0.f, -1.f}, 2.0f, 10.f, wepdata});

        Global::get().e->allObjects.push_back(projectile);

        BOOST_CHECK(character->getCurrentState().health == 100.f);

        for (float t = 0.f; t <= 9.f; t += 0.016f) {
            Global::get().e->dynamicsWorld->stepSimulation(0.016f, 0, 0);
            projectile->tick(0.016f);
        }

        BOOST_CHECK(projectile->getPosition().z < 10.f);

        BOOST_CHECK(character->getCurrentState().health < 100.f);

        Global::get().e->destroyObjectQueued(character);
        Global::get().e->destroyQueuedObjects();
    }
}
#endif

BOOST_AUTO_TEST_SUITE_END()
