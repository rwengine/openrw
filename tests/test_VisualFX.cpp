#include <boost/test/unit_test.hpp>
#include <render/VisualFX.hpp>

BOOST_AUTO_TEST_SUITE(VisualFXTests)

BOOST_AUTO_TEST_CASE(test_light_data) {
    auto fx = std::make_unique<LightFX>();

    BOOST_CHECK_EQUAL(fx->getType(), Light);
}

BOOST_AUTO_TEST_SUITE_END()
