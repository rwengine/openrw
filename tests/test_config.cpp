#include <GameConfig.hpp>
#include <boost/test/unit_test.hpp>
#include <fstream>

BOOST_AUTO_TEST_SUITE(ConfigTests)

BOOST_AUTO_TEST_CASE(test_loading) {
    // Write out a temporary file
    std::ofstream test_config("/tmp/openrw_test.ini");
    test_config << "[game]\n"
                << "path=/dev/test" << std::endl;

    GameConfig config("openrw_test.ini", "/tmp");

    BOOST_CHECK(config.isValid());

    BOOST_CHECK_EQUAL(config.getGameDataPath(), "/dev/test");
}

BOOST_AUTO_TEST_SUITE_END()
