#define BOOST_TEST_MODULE openrw
#include <boost/test/unit_test.hpp>
#include <boost/program_options.hpp>

#include "test_Globals.hpp"

#include <iostream>

static bool global_texture_ran = false;

class RWTestGlobalFixture {
public:
    RWTestGlobalFixture() {
        int argc = utf::framework::master_test_suite().argc;
        char** argv= utf::framework::master_test_suite().argv;
        parse_args(argc, argv);

        global_texture_ran = true;
    }
private:
    void parse_args(int argc, char** argv) {
        namespace po = boost::program_options;
        po::options_description test_options("Test options");
        test_options.add_options()(
            "nodata", "Disable tests that need game data")(
            "help", "Show this help message");

        po::variables_map vm;
        try {
            po::store(po::parse_command_line(argc, argv, test_options), vm);
            po::notify(vm);
        } catch (po::error& ex) {
            std::cout << "Error parsing arguments: " << ex.what() << std::endl;
            std::cerr << test_options;
            _exit(EXIT_FAILURE);
        }
        if (vm.count("help")) {
            std::cout << test_options;
            _exit(EXIT_SUCCESS);
        }

        if (vm.count("nodata")) {
            global_args.with_data = false;
        }
    }
};

BOOST_TEST_GLOBAL_FIXTURE(RWTestGlobalFixture);

BOOST_AUTO_TEST_CASE(test_global_fixture) {
    BOOST_TEST_REQUIRE(global_texture_ran);
}
