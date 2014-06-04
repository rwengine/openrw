#include <boost/test/unit_test.hpp>
#include "test_globals.hpp"
#include <render/Model.hpp>

BOOST_AUTO_TEST_SUITE(LoaderDFFTests)

BOOST_AUTO_TEST_CASE(test_open_dff)
{
	LoaderIMG loader;

	BOOST_REQUIRE( loader.load(Global::getGamePath() + "/models/gta3") );

	auto d = loader.loadToMemory("landstal.dff");
	
	LoaderDFF dffloader;
	Model* m = dffloader.loadFromMemory(d, &Global::get().e->gameData);
	
	BOOST_REQUIRE( m != nullptr );
	
	BOOST_CHECK( m->frames.size() > 0 );
	
	delete d;
	delete m;
}

BOOST_AUTO_TEST_SUITE_END()
