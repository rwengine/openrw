#include <boost/test/unit_test.hpp>
#include <data/Clump.hpp>
#include <platform/FileHandle.hpp>
#include "test_Globals.hpp"

BOOST_AUTO_TEST_SUITE(LoaderDFFTests)

#if RW_TEST_WITH_DATA
BOOST_AUTO_TEST_CASE(test_load_dff) {
    {
        auto d = Global::get().e->data->index.openFile("landstal.dff");

        LoaderDFF loader;

        auto m = loader.loadFromMemory(d);

        BOOST_REQUIRE(m != nullptr);

        BOOST_REQUIRE(m->getFrame());

        BOOST_REQUIRE(!m->getAtomics().empty());
        const auto& atomic = m->getAtomics()[0];

        BOOST_REQUIRE(atomic->getGeometry());
        BOOST_REQUIRE(atomic->getFrame());
    }
}

#endif

BOOST_AUTO_TEST_CASE(test_clump_clone) {
    {
        auto frame1 = std::make_shared<ModelFrame>(0);
        frame1->setName("Frame1");
        auto frame2 = std::make_shared<ModelFrame>(1);
        frame2->setName("Frame2");

        frame1->addChild(frame2);

        auto geometry = std::make_shared<Geometry>();

        auto atomic = std::make_shared<Atomic>();
        atomic->setFrame(frame2);
        atomic->setGeometry(geometry);

        auto clump = std::make_shared<Clump>();
        clump->addAtomic(atomic);
        clump->setFrame(frame1);

        // Now clone and verify that:
        //  The hierarchy has the same data but different objects
        //  The atomics have the same data but different objects
        // Suspected correct behaviour:
        //  The geometry is the same for each atomic

        auto newclump = std::shared_ptr<Clump>(clump->clone());
        BOOST_REQUIRE(newclump);

        BOOST_CHECK_NE(newclump, clump);

        BOOST_REQUIRE(newclump->getFrame());
        BOOST_CHECK_NE(newclump->getFrame(), clump->getFrame());
        BOOST_CHECK_EQUAL(newclump->getFrame()->getChildren().size(), 1);

        BOOST_CHECK_EQUAL(newclump->getAtomics().size(), 1);

        BOOST_CHECK_EQUAL(frame1->getName(), newclump->getFrame()->getName());
    }
}

BOOST_AUTO_TEST_SUITE_END()
