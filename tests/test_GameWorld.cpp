#include <boost/test/unit_test.hpp>
#include <engine/GameData.hpp>
#include <engine/GameWorld.hpp>
#include <objects/InstanceObject.hpp>
#include "test_Globals.hpp"

BOOST_AUTO_TEST_SUITE(GameWorldTests)

#if RW_TEST_WITH_DATA
BOOST_AUTO_TEST_CASE(test_gameobject_id) {
    GameWorld gw(&Global::get().log, Global::get().d);

    auto object1 = gw.createInstance(1337, glm::vec3(100.f, 0.f, 0.f));
    auto object2 = gw.createInstance(1337, glm::vec3(100.f, 0.f, 100.f));

    BOOST_CHECK_NE(object1->getGameObjectID(), object2->getGameObjectID());
}

BOOST_AUTO_TEST_CASE(test_offsetgametime) {
    GameWorld gw(&Global::get().log, Global::get().d);
    gw.state = new GameState();

    BOOST_CHECK_EQUAL(0, gw.getHour());
    BOOST_CHECK_EQUAL(0, gw.getMinute());

    gw.offsetGameTime(30);
    BOOST_CHECK_EQUAL(0, gw.getHour());
    BOOST_CHECK_EQUAL(30, gw.getMinute());

    gw.offsetGameTime(30);
    BOOST_CHECK_EQUAL(1, gw.getHour());
    BOOST_CHECK_EQUAL(0, gw.getMinute());

    gw.offsetGameTime(-30);
    BOOST_CHECK_EQUAL(0, gw.getHour());
    BOOST_CHECK_EQUAL(30, gw.getMinute());

    gw.offsetGameTime(-60);
    BOOST_CHECK_EQUAL(23, gw.getHour());
    BOOST_CHECK_EQUAL(30, gw.getMinute());

    gw.offsetGameTime(30);
    BOOST_CHECK_EQUAL(0, gw.getHour());
    BOOST_CHECK_EQUAL(0, gw.getMinute());

    gw.offsetGameTime(24 * 60);
    BOOST_CHECK_EQUAL(0, gw.getHour());
    BOOST_CHECK_EQUAL(0, gw.getMinute());

    gw.offsetGameTime(8 * 60 + 25);
    BOOST_CHECK_EQUAL(8, gw.getHour());
    BOOST_CHECK_EQUAL(25, gw.getMinute());

    gw.offsetGameTime(-30);
    BOOST_CHECK_EQUAL(7, gw.getHour());
    BOOST_CHECK_EQUAL(55, gw.getMinute());

    gw.offsetGameTime(-24 * 60);
    BOOST_CHECK_EQUAL(7, gw.getHour());
    BOOST_CHECK_EQUAL(55, gw.getMinute());

    gw.offsetGameTime(0);
    BOOST_CHECK_EQUAL(7, gw.getHour());
    BOOST_CHECK_EQUAL(55, gw.getMinute());

    gw.offsetGameTime(30 * 24 * 60 + 90);
    BOOST_CHECK_EQUAL(9, gw.getHour());
    BOOST_CHECK_EQUAL(25, gw.getMinute());
}
#endif

BOOST_AUTO_TEST_SUITE_END()
