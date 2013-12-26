#include <boost/test/unit_test.hpp>
#include "test_globals.hpp"
#include <State.hpp>

BOOST_AUTO_TEST_SUITE(StateUnitTests)

BOOST_AUTO_TEST_CASE(state_test_generic)
{
	bool entered = false;
	bool exited = false;
	bool ticked = false;
	
	GenericState ls(
		[&](State*) { entered = true; },
		[&](State*, float) { ticked = true; },
		[&](State*) { exited = true; },
		[](State*, const sf::Event&){}
				 );
	
	ls.enter();
	
	BOOST_CHECK( entered );
	
	ls.tick(1.f);
	
	BOOST_CHECK( ticked );
	
	ls.exit();
	
	BOOST_CHECK( exited );
}

BOOST_AUTO_TEST_CASE(state_test_switch)
{
	bool entered = false;
	bool exited = false;
	bool ticked = false;
	
	GenericState ls(
		[&](State*) { entered = true; },
		[&](State*, float) { ticked = true; },
		[&](State*) { exited = true; },
		[](State*, const sf::Event&){}
				 );
	
	StateManager::get().enter(&ls);
	
	BOOST_CHECK( entered );
	
	StateManager::get().tick(1.f);
	
	BOOST_CHECK( ticked );
	
	StateManager::get().exit();
	
	BOOST_CHECK( exited );
}

BOOST_AUTO_TEST_SUITE_END()
