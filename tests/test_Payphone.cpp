#include <boost/test/unit_test.hpp>
#include <ai/PlayerController.hpp>
#include <engine/GameWorld.hpp>
#include <engine/Payphone.hpp>
#include <objects/CharacterObject.hpp>
#include <objects/GameObject.hpp>
#include <objects/InstanceObject.hpp>
#include "test_Globals.hpp"
#if RW_TEST_WITH_DATA

BOOST_AUTO_TEST_SUITE(PayphoneTests)

BOOST_AUTO_TEST_CASE(test_payphone_interaction) {
    {
        const auto playerID = 7777;
        auto character = Global::get().e->createPlayer(
            {0.f, 0.f, 0.f}, {1.f, 0.f, 0.f, 0.f}, playerID);
        BOOST_REQUIRE(character != nullptr);

        Global::get().e->state->playerObject = playerID;

        // phonebooth1 from ipl file
        const auto modelID = 1335;
        auto payphoneObj = Global::get().e->createInstance(
            modelID, {1.f, 0.f, 0.f}, {1.f, 0.f, 0.f, 0.f});
        BOOST_REQUIRE(payphoneObj != nullptr);

        auto payphone = Global::get().e->createPayphone({1.f, 0.f});
        BOOST_REQUIRE(payphone != nullptr);
        payphone->setMessageAndStartRinging("");

        auto dt = 0.016f;
        Global::get().e->state->gameTime += dt;
        character->tick(dt);
        payphone->tick(dt);

        BOOST_CHECK(!Global::get().e->getPlayer()->isPickingUpPayphone());

        character->setPosition(character->getPosition() + glm::vec3{0.001f, 0.f, 0.f});

        dt = 0.016f;
        Global::get().e->state->gameTime += dt;
        // character->tick(dt);
        payphone->tick(dt);

        BOOST_CHECK(Global::get().e->getPlayer()->isPickingUpPayphone());
        BOOST_CHECK(!payphone->isTalking());

        dt = 10.f;
        Global::get().e->state->gameTime += dt;
        character->tick(dt);
        payphone->tick(dt);

        BOOST_CHECK(Global::get().e->getPlayer()->isTalkingOnPayphone());
        BOOST_CHECK(payphone->isTalking());

        dt = 3.f;
        Global::get().e->state->gameTime += dt;
        character->tick(dt);
        payphone->tick(dt);

        BOOST_CHECK(Global::get().e->getPlayer()->isHangingUpPayphone());
        BOOST_CHECK(!payphone->isTalking());

        Global::get().e->destroyObject(payphoneObj);
        Global::get().e->destroyObject(character);
    }
}

BOOST_AUTO_TEST_SUITE_END()

#endif
