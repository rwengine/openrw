#include <boost/test/unit_test.hpp>
#include <objects/CharacterObject.hpp>
#include <objects/VehicleObject.hpp>
#include <ai/DefaultAIController.hpp>
#include "test_globals.hpp"
#include <engine/Animator.hpp>

BOOST_AUTO_TEST_SUITE(CharacterTests)

BOOST_AUTO_TEST_CASE(test_create)
{
	{
		auto character = Global::get().e->createPedestrian(1, {100.f, 100.f, 50.f});

		BOOST_REQUIRE( character != nullptr );

		auto controller = new DefaultAIController(character);


		// Check the initial activity is Idle.
		BOOST_CHECK_EQUAL( controller->getCurrentActivity(), nullptr );

		// Check that Idle activities are instantly displaced.
		controller->setNextActivity( new Activities::GoTo( glm::vec3{ 1000.f, 0.f, 0.f } ) );

		BOOST_CHECK_EQUAL( controller->getCurrentActivity()->name(), "GoTo" );
		BOOST_CHECK_EQUAL( controller->getNextActivity(), nullptr );

		Global::get().e->destroyObject(character);
		delete controller;
	}
}

BOOST_AUTO_TEST_CASE(test_activities)
{
	{
		auto character = Global::get().e->createPedestrian(1, {0.f, 0.f, 225.6f});

		BOOST_REQUIRE( character != nullptr );

		auto controller = new DefaultAIController(character);

		controller->setNextActivity( new Activities::GoTo( glm::vec3{ 10.f, 10.f, 0.f } ) );

		BOOST_CHECK_EQUAL( controller->getCurrentActivity()->name(), "GoTo" );

		for(float t = 0.f; t < 11.5f; t+=(1.f/60.f)) {
			controller->update(1.f/60.f);
			character->tick(1.f/60.f);
			Global::get().e->dynamicsWorld->stepSimulation(1.f/60.f);
		}

		// This check will undoubtably break in the future, please improve.
		BOOST_CHECK_CLOSE( glm::distance(character->getPosition(), {10.f, 10.f, 0.f}), 1.0f, 100.0f );

		Global::get().e->destroyObject(character);
		delete controller;
	}
	{
		VehicleObject* vehicle = Global::get().e->createVehicle(90u, glm::vec3(10.f, 0.f, 0.f), glm::quat());
		BOOST_REQUIRE(vehicle != nullptr);
		BOOST_REQUIRE(vehicle->model != nullptr);

		auto character = Global::get().e->createPedestrian(1, {0.f, 0.f, 0.f});

		BOOST_REQUIRE( character != nullptr );

		auto controller = new DefaultAIController(character);

		controller->setNextActivity( new Activities::EnterVehicle( vehicle, 0 ) );

		for(float t = 0.f; t < 0.5f; t+=(1.f/60.f)) {
			character->tick(1.f/60.f);
			Global::get().e->dynamicsWorld->stepSimulation(1.f/60.f);
		}

		BOOST_CHECK_EQUAL( nullptr, character->getCurrentVehicle() );

		for(float t = 0.f; t < 9.0f; t+=(1.f/60.f)) {
			character->tick(1.f/60.f);
			Global::get().e->dynamicsWorld->stepSimulation(1.f/60.f);
		}

		BOOST_CHECK_EQUAL( vehicle, character->getCurrentVehicle() );

		controller->setNextActivity( new Activities::ExitVehicle( ) );

		for(float t = 0.f; t < 9.0f; t+=(1.f/60.f)) {
			character->tick(1.f/60.f);
			Global::get().e->dynamicsWorld->stepSimulation(1.f/60.f);
		}

		BOOST_CHECK_EQUAL( nullptr, character->getCurrentVehicle() );

		character->setPosition(glm::vec3(5.f, 0.f, 0.f));
		controller->setNextActivity( new Activities::EnterVehicle( vehicle, 0 ) );

		for(float t = 0.f; t < 0.5f; t+=(1.f/60.f)) {
			character->tick(1.f/60.f);
			Global::get().e->dynamicsWorld->stepSimulation(1.f/60.f);
		}

		BOOST_CHECK_EQUAL( nullptr, character->getCurrentVehicle() );
		controller->skipActivity();

		for(float t = 0.f; t < 5.0f; t+=(1.f/60.f)) {
			character->tick(1.f/60.f);
			Global::get().e->dynamicsWorld->stepSimulation(1.f/60.f);
		}

		BOOST_CHECK_EQUAL( nullptr, character->getCurrentVehicle() );

		Global::get().e->destroyObject(character);
		delete controller;
	}
}

BOOST_AUTO_TEST_CASE(test_death)
{
	{
		auto character = Global::get().e->createPedestrian(1, {100.f, 100.f, 50.f});
		BOOST_REQUIRE( character != nullptr );
		auto controller = new DefaultAIController(character);

		BOOST_CHECK_EQUAL( character->getCurrentState().health, 100.f );
		BOOST_CHECK( character->isAlive() );

		GameObject::DamageInfo dmg;
		dmg.type = GameObject::DamageInfo::Bullet;
		dmg.hitpoints = character->getCurrentState().health + 1.f;

		// Do some damage
		BOOST_CHECK(character->takeDamage(dmg));

		BOOST_CHECK( ! character->isAlive() );

		character->tick(0.16f);

		BOOST_CHECK_EQUAL(
					character->animator->getAnimation(0),
					character->animations.ko_shot_front);

		Global::get().e->destroyObject(character);
		delete controller;
	}
}

BOOST_AUTO_TEST_SUITE_END()

