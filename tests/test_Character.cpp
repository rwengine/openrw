#include "test_Globals.hpp"

#include <ai/DefaultAIController.hpp>
#include <engine/Animator.hpp>
#include <objects/CharacterObject.hpp>
#include <objects/VehicleObject.hpp>

#include <btBulletDynamicsCommon.h>

BOOST_AUTO_TEST_SUITE(CharacterTests)

BOOST_AUTO_TEST_CASE(test_create,
                     * utf::precondition(with_data{})) {
    {
        auto character =
            Global::get().e->createPedestrian(1, {100.f, 100.f, 50.f});
        BOOST_REQUIRE(character != nullptr);

        auto controller = character->controller;
        BOOST_REQUIRE(controller != nullptr);

        // Check the initial activity is Idle.
        BOOST_CHECK_EQUAL(controller->getCurrentActivity(), nullptr);

        // Check that Idle activities are instantly displaced.
        controller->setNextActivity(
            std::make_unique<Activities::GoTo>(glm::vec3{1000.f, 0.f, 0.f}));

        BOOST_CHECK_EQUAL(controller->getCurrentActivity()->name(), "GoTo");
        BOOST_CHECK_EQUAL(controller->getNextActivity(), nullptr);

        Global::get().e->destroyObject(character);
    }
}

BOOST_AUTO_TEST_CASE(test_activities,
                     * utf::precondition(with_data{})) {
    {
        auto character =
            Global::get().e->createPedestrian(1, {0.f, 0.f, 225.6f});
        BOOST_REQUIRE(character != nullptr);

        auto controller = character->controller;
        BOOST_REQUIRE(controller != nullptr);

        controller->setNextActivity(
            std::make_unique<Activities::GoTo>(glm::vec3{10.f, 10.f, 0.f}));

        BOOST_CHECK_EQUAL(controller->getCurrentActivity()->name(), "GoTo");

        for (float t = 0.f; t < 11.5f; t += (1.f / 60.f)) {
            controller->update(1.f / 60.f);
            character->tick(1.f / 60.f);
            Global::get().e->dynamicsWorld->stepSimulation(1.f / 60.f);
        }

        // Actually GoTo ignores z axis (up)
        BOOST_CHECK_LT(glm::distance(glm::vec2{character->getPosition()},
                                        {10.f, 10.f}),
                                        0.1f);

        Global::get().e->destroyObject(character);
    }
    {
        VehicleObject* vehicle = Global::get().e->createVehicle(
            90u, glm::vec3(10.f, 0.f, 0.f), glm::quat{1.0f,0.0f,0.0f,0.0f});
        BOOST_REQUIRE(vehicle != nullptr);
        BOOST_REQUIRE(vehicle->getModel() != nullptr);

        auto character = Global::get().e->createPedestrian(1, {0.f, 0.f, 0.f});
        BOOST_REQUIRE(character != nullptr);

        auto controller = character->controller;
        BOOST_REQUIRE(controller != nullptr);

        controller->setNextActivity(
            std::make_unique<Activities::EnterVehicle>(vehicle, 0));

        for (float t = 0.f; t < 0.5f; t += (1.f / 60.f)) {
            character->tick(1.f / 60.f);
            Global::get().e->dynamicsWorld->stepSimulation(1.f / 60.f);
        }

        BOOST_CHECK_EQUAL(nullptr, character->getCurrentVehicle());

        for (float t = 0.f; t < 9.0f; t += (1.f / 60.f)) {
            character->tick(1.f / 60.f);
            Global::get().e->dynamicsWorld->stepSimulation(1.f / 60.f);
        }

        BOOST_CHECK_EQUAL(vehicle, character->getCurrentVehicle());

        controller->setNextActivity(
            std::make_unique<Activities::ExitVehicle>());

        for (float t = 0.f; t < 9.0f; t += (1.f / 60.f)) {
            character->tick(1.f / 60.f);
            Global::get().e->dynamicsWorld->stepSimulation(1.f / 60.f);
        }

        BOOST_CHECK_EQUAL(nullptr, character->getCurrentVehicle());

        character->setPosition(glm::vec3(5.f, 0.f, 0.f));
        controller->setNextActivity(
            std::make_unique<Activities::EnterVehicle>(vehicle, 0));

        for (float t = 0.f; t < 0.5f; t += (1.f / 60.f)) {
            character->tick(1.f / 60.f);
            Global::get().e->dynamicsWorld->stepSimulation(1.f / 60.f);
        }

        BOOST_CHECK_EQUAL(nullptr, character->getCurrentVehicle());
        controller->skipActivity();

        for (float t = 0.f; t < 5.0f; t += (1.f / 60.f)) {
            character->tick(1.f / 60.f);
            Global::get().e->dynamicsWorld->stepSimulation(1.f / 60.f);
        }

        BOOST_CHECK_EQUAL(nullptr, character->getCurrentVehicle());

        Global::get().e->destroyObject(character);
    }
}

BOOST_AUTO_TEST_CASE(test_death,
                     * utf::precondition(with_data{})) {
    {
        auto character =
            Global::get().e->createPedestrian(1, {100.f, 100.f, 50.f});
        BOOST_REQUIRE(character != nullptr);

        BOOST_CHECK_EQUAL(character->getCurrentState().health, 100.f);
        BOOST_CHECK(character->isAlive());

        GameObject::DamageInfo dmg;
        dmg.type = GameObject::DamageInfo::Bullet;
        dmg.hitpoints = character->getCurrentState().health + 1.f;

        // Do some damage
        BOOST_CHECK(character->takeDamage(dmg));

        BOOST_CHECK(!character->isAlive());

        character->tick(0.16f);

        BOOST_CHECK_EQUAL(
            character->animator->getAnimation(0),
            character->animations->animation(AnimCycle::KnockOutShotFront0));

        Global::get().e->destroyObject(character);
    }
}

BOOST_AUTO_TEST_CASE(test_cycle_animating,
                     * utf::precondition(with_data{})) {
    {
        auto character =
            Global::get().e->createPedestrian(1, {100.f, 100.f, 50.f});

        BOOST_REQUIRE(character != nullptr);

        // Set the character cycle
        character->playCycle(AnimCycle::ArrestGun);

        BOOST_CHECK_EQUAL(static_cast<uint32_t>(character->getCurrentCycle()),
                          static_cast<uint32_t>(AnimCycle::ArrestGun));
    }
}

BOOST_AUTO_TEST_SUITE_END()
