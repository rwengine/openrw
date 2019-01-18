#include <boost/test/unit_test.hpp>
#include <data/CutsceneData.hpp>
#include <loaders/LoaderCutsceneDAT.hpp>
#include <platform/FileHandle.hpp>
#include "test_Globals.hpp"

BOOST_AUTO_TEST_SUITE(CutsceneTests, DATA_TEST_PREDICATE)

BOOST_AUTO_TEST_CASE(test_load) {
    {
        auto d = Global::get().e->data->index.openFile("intro.dat");

        CutsceneTracks tracks;

        LoaderCutsceneDAT loader;

        loader.load(tracks, d);

        BOOST_CHECK(tracks.position.find(0.f) != tracks.position.end());
        BOOST_CHECK(tracks.position.find(64.8f) != tracks.position.end());

        BOOST_CHECK(tracks.zoom.find(64.8f) != tracks.zoom.end());

        BOOST_CHECK(tracks.zoom.find(64.8f) != tracks.zoom.end());

        BOOST_CHECK(tracks.target.find(64.8f) != tracks.target.end());

        BOOST_CHECK(tracks.duration == 64.8f);
    }
}

BOOST_AUTO_TEST_SUITE_END()
