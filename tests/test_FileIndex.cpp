#include <boost/test/unit_test.hpp>
#include <platform/FileHandle.hpp>
#include <platform/FileIndex.hpp>
#include "test_Globals.hpp"

BOOST_AUTO_TEST_SUITE(FileIndexTests)

BOOST_AUTO_TEST_CASE(test_normalizeName) {
    std::string ref = "a/b/c";
    {
        std::string dirty = "a\\b\\c";
        BOOST_CHECK_EQUAL(ref, FileIndex::normalizeFilePath(dirty));
    }
    {
        std::string dirty = "A/B/C";
        BOOST_CHECK_EQUAL(ref, FileIndex::normalizeFilePath(dirty));
    }
    {
        std::string dirty = "A\\B\\C";
        BOOST_CHECK_EQUAL(ref, FileIndex::normalizeFilePath(dirty));
    }
}

BOOST_AUTO_TEST_CASE(test_indexTree, DATA_TEST_PREDICATE) {
    FileIndex index;
    index.indexTree(Global::getGamePath());

    {
        std::string upperpath{"DATA/CULLZONE.DAT"};
        auto truepath = index.findFilePath(upperpath);
        BOOST_ASSERT(!truepath.empty());
        BOOST_CHECK(upperpath != truepath);
        rwfs::path expected{Global::getGamePath()};
        expected /= "data/CULLZONE.DAT";
        BOOST_CHECK(expected.compare(expected) == 0);
    }
    {
        std::string upperpath{"DATA/MAPS/COMNBTM/COMNBTM.IPL"};
        auto truepath = index.findFilePath(upperpath);
        BOOST_ASSERT(!truepath.empty());
        BOOST_CHECK(upperpath != truepath);
        rwfs::path expected{Global::getGamePath()};
        expected /= "data/maps/comnbtm/comNbtm.ipl";
        BOOST_CHECK(expected.compare(truepath) == 0);
    }
}

BOOST_AUTO_TEST_CASE(test_openFile, DATA_TEST_PREDICATE) {
    FileIndex index;
    index.indexTree(Global::getGamePath() + "/data");

    auto handle = index.openFile("cullzone.dat");
    BOOST_CHECK(handle.data != nullptr);
}

BOOST_AUTO_TEST_CASE(test_indexArchive, DATA_TEST_PREDICATE) {
    FileIndex index;
    index.indexTree(Global::getGamePath());

    {
        auto handle = index.openFile("landstal.dff");
        BOOST_CHECK(handle.data == nullptr);
    }

    index.indexArchive("models/gta3.img");

    {
        auto handle = index.openFile("landstal.dff");
        BOOST_CHECK(handle.data != nullptr);
    }
}

BOOST_AUTO_TEST_SUITE_END()
