#include <boost/test/unit_test.hpp>
#include <loaders/LoaderIPL.hpp>
#include "test_globals.hpp"

BOOST_AUTO_TEST_SUITE(LoaderIPLTests)

#if RW_TEST_WITH_DATA
BOOST_AUTO_TEST_CASE(test_load_zones) {
    LoaderIPL loader;
    const auto& gdpath = Global::get().getGamePath();
    BOOST_REQUIRE(loader.load(gdpath + "/data/gta3.zon"));

    BOOST_REQUIRE(loader.zones.size() > 2);

    auto& zone1 = loader.zones[0];
    BOOST_CHECK_EQUAL(zone1.name, "ROADBR1");
}

#endif

BOOST_AUTO_TEST_SUITE_END()
