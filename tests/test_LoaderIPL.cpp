#include <boost/test/unit_test.hpp>
#include <loaders/LoaderIPL.hpp>
#include "test_Globals.hpp"

struct WithLoaderIPL {
    LoaderIPL loader;

    std::ifstream test_data_stream {Global::get().getGamePath() + "/data/gta3.zon"};
};

BOOST_FIXTURE_TEST_SUITE(LoaderIPLTests, WithLoaderIPL)

#if RW_TEST_WITH_DATA
BOOST_AUTO_TEST_CASE(zone_count_is_correct) {
    BOOST_REQUIRE(loader.load(test_data_stream));

    BOOST_TEST(loader.zones.size() == 42);
}

BOOST_AUTO_TEST_CASE(zone_data_is_correct) {
    BOOST_REQUIRE(loader.load(test_data_stream));

    auto& zone1 = loader.zones[1];
    BOOST_TEST(zone1.name == "PORT_W");
    BOOST_TEST(zone1.type == 0);
    BOOST_TEST(zone1.min.x == 751.68f);
    BOOST_TEST(zone1.island == 1);
}
#endif

BOOST_AUTO_TEST_SUITE_END()
