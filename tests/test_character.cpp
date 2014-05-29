#include <boost/test/unit_test.hpp>
#include <objects/GTACharacter.hpp>
#include <ai/GTADefaultAIController.hpp>
#include "test_globals.hpp"

BOOST_AUTO_TEST_SUITE(CharacterTests)

BOOST_AUTO_TEST_CASE(test_create)
{
	{
		auto character = Global::get().e->createPedestrian(1, {100.f, 100.f, 50.f});

		BOOST_REQUIRE( character != nullptr );

		auto controller = new GTADefaultAIController(character);

		// Check the initial activity is Idle.
		BOOST_CHECK_EQUAL( controller->getCurrentActivity(), GTAAIController::Idle );

		// Check that Idle activities are instantly displaced.
		controller->setNextActivity(GTAAIController::GoTo, glm::vec3{ 1000.f, 0.f, 0.f });
		BOOST_CHECK_EQUAL( controller->getCurrentActivity(), GTAAIController::GoTo );
		BOOST_CHECK_EQUAL( controller->getNextActivity(), GTAAIController::Idle );

		Global::get().e->destroyObject(character);
		delete controller;
	}
	{
		GTAAIController::ActivityParameter ap { glm::vec3 { 1.f, 2.f, 3.f } };

		BOOST_CHECK_EQUAL( ap.position, glm::vec3( 1.f, 2.f, 3.f ) );
	}
}

BOOST_AUTO_TEST_CASE(test_activities)
{
	{
		auto character = Global::get().e->createPedestrian(1, {0.f, 0.f, 0.f});

		BOOST_REQUIRE( character != nullptr );

		auto controller = new GTADefaultAIController(character);

		controller->setNextActivity( GTAAIController::GoTo, { glm::vec3{ 10.f, 0.f, 0.f } } );

		BOOST_CHECK_EQUAL( controller->getCurrentActivity(), GTAAIController::GoTo );
		BOOST_CHECK_EQUAL( controller->getCurrentActivityParameter().position, glm::vec3( 10.f, 0.f, 0.f ) );

		for(float t = 0.f; t < 11.f; t+=(1.f/60.f)) {
			controller->update(1.f/60.f);
			character->tick(1.f/60.f);
			Global::get().e->dynamicsWorld->stepSimulation(1.f/60.f);
		}

		// This check will undoubtably break in the future, please improve.
		BOOST_CHECK_CLOSE( glm::distance(character->getPosition(), {10.f, 0.f, 0.f}), 0.038f, 1.0f );

		Global::get().e->destroyObject(character);
		delete controller;
	}
}

BOOST_AUTO_TEST_SUITE_END()

