#include <boost/test/unit_test.hpp>
#include <data/Chase.hpp>
#include "test_Globals.hpp"

BOOST_AUTO_TEST_SUITE(ChaseTests)

#if RW_TEST_WITH_DATA
BOOST_AUTO_TEST_CASE(test_load_keyframes) {
    std::vector<ChaseKeyframe> keyframes;
    BOOST_REQUIRE(ChaseKeyframe::load(
        Global::getGamePath() + "/data/paths/CHASE0.DAT", keyframes));
    BOOST_REQUIRE(keyframes.size() == 5400);
    BOOST_CHECK_CLOSE(keyframes[0].position.x, 273.5422, 0.1);
}
#endif

BOOST_AUTO_TEST_SUITE_END()
