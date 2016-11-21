#include <boost/test/unit_test.hpp>
#include <loaders/LoaderIMG.hpp>
#include "test_globals.hpp"

BOOST_AUTO_TEST_SUITE(ArchiveTests)

#if RW_TEST_WITH_DATA
BOOST_AUTO_TEST_CASE(test_open_archive) {
    LoaderIMG archive;

    BOOST_REQUIRE(archive.load(Global::getGamePath() + "/models/gta3"));

    BOOST_CHECK(archive.getAssetCount() > 0);

    LoaderIMGFile f, f2;

    f = archive.getAssetInfoByIndex(0);

    // A few assumptions..

    BOOST_CHECK_EQUAL(f.name, "radar00.txd");
    BOOST_CHECK_EQUAL(f.offset, 0);
    BOOST_CHECK_EQUAL(f.size, 33);

    BOOST_CHECK(archive.findAssetInfo("radar00.txd", f2));

    BOOST_CHECK_EQUAL(f2.name, f.name);
    BOOST_CHECK_EQUAL(f2.offset, f.offset);
    BOOST_CHECK_EQUAL(f2.size, f.size);
}
#endif

BOOST_AUTO_TEST_SUITE_END()
