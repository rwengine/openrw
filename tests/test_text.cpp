#include <boost/test/unit_test.hpp>
#include "test_globals.hpp"
#include <data/GameTexts.hpp>
#include <loaders/LoaderGXT.hpp>

BOOST_AUTO_TEST_SUITE(TextTests)

BOOST_AUTO_TEST_CASE(load_test)
{
	{
		auto d = Global::get().e->gameData.openFile("english.gxt");

		GameTexts texts;

		LoaderGXT loader;

		loader.load( texts, d );

		BOOST_CHECK_EQUAL( texts.text("1008"), "BUSTED" );
	}
}

BOOST_AUTO_TEST_SUITE_END()
