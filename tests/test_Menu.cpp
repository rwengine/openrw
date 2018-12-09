#include <boost/test/unit_test.hpp>

#include <MenuSystem.hpp>

BOOST_AUTO_TEST_SUITE(MenuTests)

BOOST_AUTO_TEST_CASE(menu_test_click) {
    bool clicked = false;
    Menu test({{"Test", [&] { clicked = true; }}});

    BOOST_CHECK(!clicked);

    // Click underneath the menu item.
    test.click(0.f, -1.f);

    BOOST_CHECK(!clicked);

    float h = 30.f;

    test.click(0.f, h + 1.f);

    BOOST_CHECK(!clicked);

    test.click(0.f, h / 2.f);

    BOOST_CHECK(clicked);
}

BOOST_AUTO_TEST_CASE(menu_test_click_offset) {
    bool clicked = false;
    Menu test({{"Test", [&] { clicked = true; }}}, glm::vec2(200.f, 200.f));
    BOOST_CHECK(!clicked);

    // Click underneath the menu item.
    test.click(201.f, -1.f);

    BOOST_CHECK(!clicked);

    float h = 30.f;

    test.click(201.f, 200.f + h + 1.f);

    BOOST_CHECK(!clicked);

    test.click(201.f, 200.f + h / 2.f);

    BOOST_CHECK(clicked);
}

BOOST_AUTO_TEST_CASE(menu_test_active_index) {
    int clickIndex = -1;
    Menu test({{"Test1", [&] { clickIndex = 0; }},
               {"Test2", [&] { clickIndex = 1; }}});

    test.activate();

    BOOST_CHECK(clickIndex == -1);

    test.move(1);
    test.activate();

    BOOST_CHECK(clickIndex == 0);

    test.move(1);
    test.activate();

    BOOST_CHECK(clickIndex == 1);

    test.move(-1);
    test.activate();

    BOOST_CHECK(clickIndex == 0);
}

BOOST_AUTO_TEST_CASE(menu_test_hover_index) {
    int clickIndex = -1;
    Menu test({{"Test1", [&] { clickIndex = 0; }},
               {"Test2", [&] { clickIndex = 1; }}});

    test.hover(0.f, 30.f - 0.1f);
    BOOST_CHECK(test.activeEntry == 0);

    test.hover(0.f, 30.f + 0.1f);
    BOOST_CHECK(test.activeEntry == 1);
}

BOOST_AUTO_TEST_SUITE_END()
