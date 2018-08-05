#include <boost/test/unit_test.hpp>
#include <data/ModelData.hpp>
#include <loaders/LoaderIDE.hpp>
#include "test_Globals.hpp"

struct WithLoaderIDE {
    LoaderIDE loader;
};

namespace {
constexpr auto kTestDataObjects = R"(
objs
1100, NAME, TXD, 1, 220, 0"
end

cars
90, vehicle, texture, car, HANDLING, NAME, richfamily, 10, 7, 0, 164, 0.8
end
)";
}

BOOST_FIXTURE_TEST_SUITE(ObjectDataTests, WithLoaderIDE)

BOOST_AUTO_TEST_CASE(parses_basic_instance) {
    std::istringstream str {kTestDataObjects};
    loader.load(str, {});

    BOOST_ASSERT(loader.objects.find(1100) != loader.objects.end());

    auto obj = loader.objects[1100].get();

    auto def = dynamic_cast<SimpleModelInfo *>(obj);

    BOOST_ASSERT(def->type() == ModelDataType::SimpleInfo);

    BOOST_CHECK_EQUAL(def->name, "NAME");
    BOOST_CHECK_EQUAL(def->textureslot, "TXD");
    BOOST_CHECK_EQUAL(def->getNumAtomics(), 1);
    BOOST_CHECK_EQUAL(def->getLodDistance(0), 220);
    BOOST_CHECK_EQUAL(def->flags, 0);
}

BOOST_AUTO_TEST_CASE(parses_vehicle) {
    std::istringstream str {kTestDataObjects};
    loader.load(str, {});

    BOOST_ASSERT(loader.objects.find(90) != loader.objects.end());

    auto obj = loader.objects[90].get();

    auto def = dynamic_cast<VehicleModelInfo*>(obj);

    BOOST_ASSERT(def->type() == ModelDataType::VehicleInfo);

    BOOST_CHECK_EQUAL(def->name, "vehicle");
    BOOST_CHECK_EQUAL(def->textureslot, "texture");
    BOOST_CHECK_EQUAL(def->vehicletype_, VehicleModelInfo::CAR);
    BOOST_CHECK_EQUAL(def->handling_, "HANDLING");
    BOOST_CHECK_EQUAL(def->vehiclename_, "NAME");
    BOOST_CHECK_EQUAL(def->vehicleclass_, VehicleModelInfo::RICHFAMILY);
    BOOST_CHECK_EQUAL(def->frequency_, 10);
    BOOST_CHECK_EQUAL(def->wheelmodel_, 164);
    BOOST_CHECK_CLOSE(def->wheelscale_, 0.8f, 0.01f);
}

BOOST_AUTO_TEST_SUITE_END()
