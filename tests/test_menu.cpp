#include <MenuSystem.hpp>
#include <boost/test/unit_test.hpp>
#include "test_globals.hpp"

BOOST_AUTO_TEST_SUITE(MenuUnitTests)

BOOST_AUTO_TEST_CASE(menu_test_click) {
    bool clickered = false;
    Menu test(0);
    test.addEntry(Menu::lambda("Test", [&] { clickered = true; }));

    BOOST_CHECK(!clickered);

    // Click underneath the menu item.
    test.click(0.f, -1.f);

    BOOST_CHECK(!clickered);

    float h = test.entries.at(0)->getHeight();

    test.click(0.f, h + 1.f);

    BOOST_CHECK(!clickered);

    test.click(0.f, h / 2.f);

    BOOST_CHECK(clickered);
}

BOOST_AUTO_TEST_CASE(menu_test_click_offset) {
    bool clickered = false;
    Menu test(0);
    test.offset = glm::vec2(200.f, 200.f);
    test.addEntry(Menu::lambda("Test", [&] { clickered = true; }));

    BOOST_CHECK(!clickered);

    // Click underneath the menu item.
    test.click(201.f, -1.f);

    BOOST_CHECK(!clickered);

    float h = test.entries.at(0)->getHeight();

    test.click(201.f, 200.f + h + 1.f);

    BOOST_CHECK(!clickered);

    test.click(201.f, 200.f + h / 2.f);

    BOOST_CHECK(clickered);
}

BOOST_AUTO_TEST_CASE(menu_test_active_index) {
    int clickindex = -1;
    Menu test(0);
    test.addEntry(Menu::lambda("Test1", [&] { clickindex = 0; }));
    test.addEntry(Menu::lambda("Test2", [&] { clickindex = 1; }));

    test.activate();

    BOOST_CHECK(clickindex == -1);

    test.move(1);
    test.activate();

    BOOST_CHECK(clickindex == 0);

    test.move(1);
    test.activate();

    BOOST_CHECK(clickindex == 1);

    test.move(-1);
    test.activate();

    BOOST_CHECK(clickindex == 0);
}

BOOST_AUTO_TEST_CASE(menu_test_hover_index) {
    int clickindex = -1;
    Menu test(0);
    test.addEntry(Menu::lambda("Test1", [&] { clickindex = 0; }));
    test.addEntry(Menu::lambda("Test2", [&] { clickindex = 1; }));

    test.hover(0.f, test.entries[0]->getHeight() - 0.1f);
    BOOST_CHECK(test.activeEntry == 0);

    test.hover(0.f, test.entries[0]->getHeight() + 0.1f);
    BOOST_CHECK(test.activeEntry == 1);
}

BOOST_AUTO_TEST_SUITE_END()
