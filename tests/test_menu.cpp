#include <boost/test/unit_test.hpp>
#include "test_globals.hpp"
#include <MenuSystem.hpp>

BOOST_AUTO_TEST_SUITE(MenuUnitTests)

BOOST_AUTO_TEST_CASE(menu_test_click)
{
	bool clickered = false;
	sf::Font f;
	Menu test(f);
	test.addEntry(Menu::lambda("Test", [&]{ clickered = true; }));
	
	BOOST_CHECK(! clickered );
	
	// Click underneath the menu item.
	test.click(0.f, -1.f);
	
	BOOST_CHECK(! clickered );
	
	float h = test.entries.at(0)->getHeight();
	
	test.click(0.f, h + 1.f);
	
	BOOST_CHECK(! clickered );
	
	test.click(0.f, h / 2.f);
	
	BOOST_CHECK( clickered );
}

BOOST_AUTO_TEST_SUITE_END()
