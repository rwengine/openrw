#define BOOST_TEST_MODULE gtfw
#include <boost/test/included/unit_test.hpp>
#include <SFML/Window.hpp>

// Many tests require OpenGL be functional, seems like a reasonable solution.

class GlobalFixture
{
public:
	sf::Window wnd;

	GlobalFixture() {
		wnd.create(sf::VideoMode(640, 360), "Testing");
	}

	~GlobalFixture() {
		wnd.close();
	}
};

BOOST_GLOBAL_FIXTURE(GlobalFixture)
