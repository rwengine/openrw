#include <boost/test/unit_test.hpp>
#include <data/ModelData.hpp>
#include <loaders/LoaderIDE.hpp>
#include "test_Globals.hpp"

namespace {
constexpr auto kTestDataObjects = R"(
objs
1100, NAME, TXD, 1, 220, 0"
end

cars
90, vehicle, texture, car, HANDLING, NAME, richfamily, 10, 7, 0, 164, 0.8
end

peds
1, mod, txd, COP, STAT_COP, man, 7f
end
)";

template<size_t N>
void ASSERT_INSTANCE_IS(BaseModelInfo& info, const char* model, const char* txd,
        const std::array<float, N>& lods, size_t flags) {
    BOOST_ASSERT(info.type() == SimpleModelInfo::kType);
    const auto& t = dynamic_cast<SimpleModelInfo&>(info);
    BOOST_CHECK_EQUAL(t.name, model);
    BOOST_CHECK_EQUAL(t.textureslot, txd);
    BOOST_CHECK_EQUAL(t.getNumAtomics(), lods.size());
    for (auto i = 0u; i < lods.size(); ++i) {
        BOOST_CHECK_EQUAL(t.getLodDistance(i), lods[i]);
    }
    BOOST_CHECK_EQUAL(t.flags, flags);
}

void ASSERT_VEHICLE_IS(BaseModelInfo& info, const char* model, const char* txd,
        VehicleModelInfo::VehicleType type, const char* handling, const char* name,
        VehicleModelInfo::VehicleClass clas_, int frequency, ModelID wheel, float wheelScale) {
    BOOST_ASSERT(info.type() == VehicleModelInfo::kType);
    const auto& t = dynamic_cast<VehicleModelInfo&>(info);
    BOOST_CHECK_EQUAL(t.name, model);
    BOOST_CHECK_EQUAL(t.textureslot, txd);
    BOOST_CHECK_EQUAL(t.vehicletype_, type);
    BOOST_CHECK_EQUAL(t.handling_, handling);
    BOOST_CHECK_EQUAL(t.vehiclename_, name);
    BOOST_CHECK_EQUAL(t.vehicleclass_, clas_);
    BOOST_CHECK_EQUAL(t.frequency_, frequency);
    BOOST_CHECK_EQUAL(t.wheelmodel_, wheel);
    BOOST_CHECK_EQUAL(t.wheelscale_, wheelScale);
}

void ASSERT_PED_IS(BaseModelInfo& info, const char* model, const char* txd,
                   PedModelInfo::PedType type, int statindex, const char* animgroup,
                   size_t carmask) {
    BOOST_ASSERT(info.type() == PedModelInfo::kType);
    const auto& t = dynamic_cast<PedModelInfo&>(info);
    BOOST_CHECK_EQUAL(t.name, model);
    BOOST_CHECK_EQUAL(t.textureslot, txd);
    BOOST_CHECK_EQUAL(t.pedtype_, type);
    BOOST_CHECK_EQUAL(t.statindex_, statindex);
    BOOST_CHECK_EQUAL(t.animgroup_, animgroup);
    BOOST_CHECK_EQUAL(t.carsmask_, carmask);
}
}

struct WithLoaderIDE {
    LoaderIDE loader;

    std::istringstream test_data_stream {kTestDataObjects};
};

BOOST_FIXTURE_TEST_SUITE(ObjectDataTests, WithLoaderIDE)

BOOST_AUTO_TEST_CASE(objects_contains_modelID) {
    loader.load(test_data_stream, {});
    BOOST_ASSERT(loader.objects.find(1100) != loader.objects.end());
}

BOOST_AUTO_TEST_CASE(instance_data_is_correct) {
    loader.load(test_data_stream, {});
    ASSERT_INSTANCE_IS<1>(*loader.objects[1100], "NAME", "TXD", {220}, 0);
}

BOOST_AUTO_TEST_CASE(vehicle_data_is_correct) {
    loader.load(test_data_stream, {});
    ASSERT_VEHICLE_IS(*loader.objects[90], "vehicle", "texture", VehicleModelInfo::CAR, "HANDLING", "NAME",
            VehicleModelInfo::RICHFAMILY, 10, 164, 0.8f);
}

BOOST_AUTO_TEST_CASE(pedestrian_data_is_correct) {
    loader.load(test_data_stream, {});
    ASSERT_PED_IS(*loader.objects[1], "mod", "txd", PedModelInfo::COP, -1, "man", 0x7f);
}

BOOST_AUTO_TEST_SUITE_END()
