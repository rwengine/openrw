#include <boost/test/unit_test.hpp>
#include <loaders/LoaderIPL.hpp>
#include "test_Globals.hpp"

struct WithLoaderIPL {
    LoaderIPL loader;
};

BOOST_FIXTURE_TEST_SUITE(LoaderIPLTests, WithLoaderIPL)

#if RW_TEST_WITH_DATA
BOOST_AUTO_TEST_CASE(test_load_zones) {
    const auto& gdpath = Global::get().getGamePath();
    BOOST_REQUIRE(loader.load(gdpath + "/data/gta3.zon"));

    BOOST_REQUIRE(loader.zones.size() > 2);

    auto& zone1 = loader.zones[0];
    BOOST_CHECK_EQUAL(zone1.name, "ROADBR1");
}

#endif

BOOST_AUTO_TEST_SUITE_END()
