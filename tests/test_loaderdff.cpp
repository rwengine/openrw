#include <boost/test/unit_test.hpp>
#include <data/Clump.hpp>
#include "test_globals.hpp"

BOOST_AUTO_TEST_SUITE(LoaderDFFTests)

#if RW_TEST_WITH_DATA
BOOST_AUTO_TEST_CASE(test_load_dff) {
    {
        auto d = Global::get().e->data->index.openFile("landstal.dff");

        LoaderDFF loader;

        Clump* m = loader.loadFromMemory(d);

        BOOST_REQUIRE(m != nullptr);

        BOOST_REQUIRE(m->getFrame());

        BOOST_REQUIRE(!m->getAtomics().empty());
        const auto& atomic = m->getAtomics()[0];

        BOOST_REQUIRE(atomic->getGeometry());
        BOOST_REQUIRE(atomic->getFrame());

        delete m;
    }
}

#endif

BOOST_AUTO_TEST_SUITE_END()
