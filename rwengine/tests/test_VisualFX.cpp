#include <boost/test/unit_test.hpp>
#include <test_globals.hpp>
#include <render/VisualFX.hpp>

BOOST_AUTO_TEST_SUITE(VisualFXTests)

BOOST_AUTO_TEST_CASE(test_light_data)
{
	VisualFX fx(VisualFX::Light);
	
	BOOST_CHECK_EQUAL(fx.getType(), VisualFX::Light);
}

BOOST_AUTO_TEST_SUITE_END()

