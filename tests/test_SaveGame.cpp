#include <boost/test/unit_test.hpp>
#include <engine/GameState.hpp>
#include <engine/SaveGame.hpp>
#include <script/ScriptMachine.hpp>
#include <test_globals.hpp>

#if 0  // Disabled until we make a start on saving the game
BOOST_AUTO_TEST_SUITE(SaveGameTests)

BOOST_AUTO_TEST_CASE(test_write_state)
{
	GameState state;

	// Set some test data
	state.world = nullptr;
	state.currentProgress = 10;
	state.maxProgress = 124;
	state.numMissions = 12;
	state.numHiddenPackages = 34;
	state.numHiddenPackagesDiscovered = 11;
	state.numUniqueJumps = 14;
	state.numRampages = 7;
	state.maxWantedLevel = 5;
	state.currentWeather = 9;
	state.overrideNextStart = true;
	state.hour = 13;
	state.minute = 32;

	// This may break due to cwd issues
	SaveGame::writeState(state, "test_savestate.sav");

	GameState loaded;
	loaded.world = nullptr;

	BOOST_REQUIRE( SaveGame::loadState(loaded, "test_savestate.sav") );
	BOOST_CHECK( loaded.world == nullptr );
	BOOST_CHECK_EQUAL( loaded.currentProgress, state.currentProgress );
	BOOST_CHECK_EQUAL( loaded.maxProgress, state.maxProgress );
	BOOST_CHECK_EQUAL( loaded.numMissions, state.numMissions );
	BOOST_CHECK_EQUAL( loaded.numHiddenPackages, state.numHiddenPackages );
	BOOST_CHECK_EQUAL( loaded.numHiddenPackagesDiscovered, state.numHiddenPackagesDiscovered );
	BOOST_CHECK_EQUAL( loaded.numUniqueJumps, state.numUniqueJumps );
	BOOST_CHECK_EQUAL( loaded.numRampages, state.numRampages );
	BOOST_CHECK_EQUAL( loaded.maxWantedLevel, state.maxWantedLevel );
	BOOST_CHECK_EQUAL( loaded.currentWeather, state.currentWeather );
	BOOST_CHECK_EQUAL( loaded.overrideNextStart, state.overrideNextStart );
	BOOST_CHECK_EQUAL( loaded.hour, state.hour );
	BOOST_CHECK_EQUAL( loaded.minute, state.minute );
	
	// Check Garage data + garage vehicle restoration
}

BOOST_AUTO_TEST_CASE(test_load_game)
{
	GameState state;
	GameWorld world();
	SCMOpcodes s;
	auto file = Global::get().d->loadSCM("main.scm");
	ScriptMachine machine(&state, file, &s);
	
	state.world = Global::get().e;
	state.script = &machine;

	BOOST_REQUIRE( SaveGame::loadGame(state, "GTA3sf1.b") );
}

BOOST_AUTO_TEST_SUITE_END()
#endif
