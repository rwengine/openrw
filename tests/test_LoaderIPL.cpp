#include <boost/test/unit_test.hpp>
#include <data/InstanceData.hpp>
#include <loaders/LoaderIPL.hpp>
#include "test_Globals.hpp"

namespace {
constexpr auto kIPLTestData = R"(
zone
ZONE_A, 1, -100.0, -200.00, -100.0, 100.0, 1000.0, 100.0, 1
ZONE_B, 0,  200.0, 10.0, -100.0, 100.0, 1000.0, 100.0, 2
end

inst
101, ModelA, 10.0, 12.0, 5.0, 1, 1, 1, 0, 0, 1, 0
112, ModelB, 10.0, 12.0, 5.0, 1, 1, 1, 0, 0, 1, 0
112, ModelB, 11.0, 12.0, 5.0, 1, 1, 1, 0, 0, 0, 1
end
)";
}

bool operator==(const ZoneData& lhs, const ZoneData& rhs) {
    return lhs.name == rhs.name && lhs.type == rhs.type && lhs.min == rhs.min &&
           lhs.max == rhs.max && lhs.island == rhs.island;
}

std::ostream& operator<<(std::ostream& str, const ZoneData& z) {
    str << "ZoneData { " << z.name << ", " << z.type << ", " << z.min << ", "
        << z.max << ", " << z.island << " }";
    return str;
}

bool operator==(const InstanceData& lhs, const InstanceData& rhs) {
    return lhs.id == rhs.id && lhs.model == rhs.model && lhs.pos == rhs.pos &&
           lhs.scale == rhs.scale && lhs.rot == rhs.rot;
}

std::ostream& operator<<(std::ostream& str, const InstanceData& i) {
    str << "InstanceData { " << i.id << ", " << i.model << ", " << i.pos << ", "
        << i.scale << ", " << glm::to_string(i.rot) << " }";
    return str;
}

struct WithLoaderIPL {
    LoaderIPL loader;

    std::istringstream test_data_stream{kIPLTestData};
};

BOOST_FIXTURE_TEST_SUITE(LoaderIPLTests, WithLoaderIPL)

BOOST_AUTO_TEST_CASE(zone_count_is_correct) {
    BOOST_REQUIRE(loader.load(test_data_stream));
    BOOST_TEST(loader.zones.size() == 2);
}

BOOST_AUTO_TEST_CASE(zone_data_is_correct) {
    BOOST_REQUIRE(loader.load(test_data_stream));

    const auto expectedZone = ZoneData("ZONE_B", 0, {200.0f, 10.0f, -100.0},
                                       {100.0f, 1000.0f, 100.0f}, 2, 0, 0);
    BOOST_TEST(loader.zones[1] == expectedZone);
}

BOOST_AUTO_TEST_CASE(instance_count_is_correct) {
    BOOST_REQUIRE(loader.load(test_data_stream));
    BOOST_TEST(loader.m_instances.size() == 3);
}

BOOST_AUTO_TEST_CASE(instance_data_is_correct) {
    BOOST_REQUIRE(loader.load(test_data_stream));

    const auto expectedInstance =
        InstanceData(112, "ModelB", {10.0f, 12.0f, 5.0f}, {1.f, 1.f, 1.f},
                     {0.0f, 0.f, 0.f, 1.0f});
    BOOST_TEST(*loader.m_instances[1] == expectedInstance);
}

BOOST_AUTO_TEST_SUITE_END()
