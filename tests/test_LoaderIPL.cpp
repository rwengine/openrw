#include <boost/test/unit_test.hpp>
#include <loaders/LoaderIPL.hpp>
#include <data/InstanceData.hpp>
#include "test_Globals.hpp"

namespace {
constexpr auto kIPLTestData = R"(
zone
ZONE_A, 1, -100.0, -200.00, -100.0, 100.0, 1000.0, 100.0, 1
ZONE_B, 0,  200.0, 10.0, -100.0, 100.0, 1000.0, 100.0, 2
end

inst
101, ModelA, 10.0, 12.0, 5.0, 1, 1, 1, 0, 0, -0.70, 0.70
112, ModelB, 10.0, 12.0, 5.0, 1, 1, 1, 0, 0, -0.70, 0.70
112, ModelB, 11.0, 12.0, 5.0, 1, 1, 1, 0, 0, 0, 1
end
)";
}

struct WithLoaderIPL {
    LoaderIPL loader;

    std::istringstream test_data_stream {kIPLTestData};
};

BOOST_FIXTURE_TEST_SUITE(LoaderIPLTests, WithLoaderIPL)

BOOST_AUTO_TEST_CASE(zone_count_is_correct) {
    BOOST_REQUIRE(loader.load(test_data_stream));
    BOOST_TEST(loader.zones.size() == 2);
}

BOOST_AUTO_TEST_CASE(zone_data_is_correct) {
    BOOST_REQUIRE(loader.load(test_data_stream));

    auto& zone1 = loader.zones[1];
    BOOST_TEST(zone1.name == "ZONE_B");
    BOOST_TEST(zone1.type == 0);
    BOOST_TEST(zone1.min.x == 200.0f);
    BOOST_TEST(zone1.island == 2);
}

BOOST_AUTO_TEST_CASE(instance_count_is_correct) {
    BOOST_REQUIRE(loader.load(test_data_stream));
    BOOST_TEST(loader.m_instances.size() == 3);
}

BOOST_AUTO_TEST_CASE(instance_data_is_correct) {
    BOOST_REQUIRE(loader.load(test_data_stream));

    auto& instance = loader.m_instances[1];
    BOOST_TEST(instance->id == 112);
    BOOST_TEST(instance->model == "ModelB");
    BOOST_TEST(instance->pos.x == 10.0f);
    BOOST_TEST(instance->pos.y == 12.0f);
    BOOST_TEST(instance->pos.z == 5.0f);
    BOOST_TEST(instance->rot.x == 0.0f);
}

BOOST_AUTO_TEST_SUITE_END()
