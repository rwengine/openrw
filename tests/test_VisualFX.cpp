#include <boost/test/unit_test.hpp>
#include <render/VisualFX.hpp>
#include <test_globals.hpp>

BOOST_AUTO_TEST_SUITE(VisualFXTests)

BOOST_AUTO_TEST_CASE(test_light_data) {
    VisualFX fx(VisualFX::Light);

    BOOST_CHECK_EQUAL(fx.getType(), VisualFX::Light);
}

BOOST_AUTO_TEST_SUITE_END()
