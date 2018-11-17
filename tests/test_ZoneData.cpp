#include <boost/test/unit_test.hpp>
#include <data/ZoneData.hpp>
#include "test_Globals.hpp"

BOOST_AUTO_TEST_SUITE(ZoneDataTests)

BOOST_AUTO_TEST_CASE(test_contains_point) {
    ZoneData zone;
    zone.min = glm::vec3(10.f, 10.f, -5.f);
    zone.max = glm::vec3(30.f, 40.f, 5.f);

    BOOST_CHECK(zone.containsPoint({15.f, 15.f, 0.f}));
    BOOST_CHECK(zone.containsPoint({10.f, 10.f, 0.f}));
    BOOST_CHECK(zone.containsPoint({30.f, 35.f, 0.f}));
    BOOST_CHECK(!zone.containsPoint({35.f, 30.f, 0.f}));
    BOOST_CHECK(!zone.containsPoint({0.f, 0.f, 0.f}));
    BOOST_CHECK(!zone.containsPoint({-15.f, -15.f, 0.f}));
}

BOOST_AUTO_TEST_CASE(test_hierarchy) {
    ZoneData zone;
    zone.min = glm::vec3(-10.f, -10.f, -5.f);
    zone.max = glm::vec3(10.f, 10.f, 5.f);
    ZoneData leaf;
    leaf.min = glm::vec3(0.f, 0.f, -5.f);
    leaf.max = glm::vec3(10.f, 10.f, 5.f);
    BOOST_CHECK(zone.insertZone(leaf));

    BOOST_CHECK_EQUAL(zone.findLeafAtPoint({-5.f, 0.f, 0.f}), &zone);
    BOOST_CHECK_EQUAL(zone.findLeafAtPoint({5.f, 5.f, 0.f}), &leaf);
}
BOOST_AUTO_TEST_SUITE_END()
