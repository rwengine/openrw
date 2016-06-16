#include <boost/test/unit_test.hpp>
#include <platform//FileIndex.hpp>
#include <test_globals.hpp>

BOOST_AUTO_TEST_SUITE(FileIndexTests)

#if RW_TEST_WITH_DATA
BOOST_AUTO_TEST_CASE(test_index)
{
	FileIndex index;

	index.indexDirectory(Global::getGamePath()+"/data");
	
	FileIndex::IndexData data;
	BOOST_CHECK( index.findFile("cullzone.dat", data) );
	BOOST_CHECK_EQUAL( data.filename, "cullzone.dat" );
	BOOST_CHECK_EQUAL( data.originalName, "CULLZONE.DAT" );
	BOOST_CHECK( data.archive.empty() );
}

BOOST_AUTO_TEST_CASE(test_file)
{
	FileIndex index;

	index.indexDirectory(Global::getGamePath()+"/data");
	
	auto handle = index.openFile("cullzone.dat");
	BOOST_CHECK( handle != nullptr );
}

BOOST_AUTO_TEST_CASE(test_index_archive)
{
	FileIndex index;

	index.indexArchive(Global::getGamePath()+"/models/gta3.img");
	
	FileIndex::IndexData data;
	BOOST_CHECK( index.findFile("landstal.dff", data) );
	BOOST_CHECK_EQUAL( data.filename, "landstal.dff" );
	BOOST_CHECK_EQUAL( data.originalName, "landstal.dff" );
	BOOST_CHECK_EQUAL( data.archive, "gta3.img" );
}

BOOST_AUTO_TEST_CASE(test_file_archive)
{
	FileIndex index;
	
	index.indexArchive(Global::getGamePath()+"/models/gta3.img");
	
	auto handle = index.openFile("landstal.dff");
	BOOST_CHECK( handle != nullptr );
}
#endif

BOOST_AUTO_TEST_SUITE_END()
