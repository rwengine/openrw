#include <boost/test/unit_test.hpp>
#include <engine/GameState.hpp>
#include <engine/SaveGame.hpp>
#include <script/ScriptMachine.hpp>
#include <engine/SaveGameSerialization.hpp>
#include <cstdint>

#include "test_Globals.hpp"

using namespace SaveGameSerialization;

BOOST_AUTO_TEST_SUITE(SaveGameTests)

struct SaveBlockFixture {
    static constexpr std::array<unsigned char, 20> kTestSaveData
    {{
         0x04, 0x00, 0x00, 0x00,
         0xFF, 0xFF, 0xFF, 0xFF,
         0x08, 0x00, 0x00, 0x00,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
     }};

    std::stringstream _truthBuff{std::ios_base::in | std::ios_base::out
                                 | std::ios_base::binary};
    Reader _reader{_truthBuff};

    SaveBlockFixture() {
        auto data = reinterpret_cast<const char*>(kTestSaveData.data());
        _truthBuff.rdbuf()->sputn(data, sizeof(kTestSaveData));

    }
};

BOOST_FIXTURE_TEST_CASE(test_serialization_loads_mutliple_blocks,
                        SaveBlockFixture) {
    {
        SaveBlock block1 {_reader};
        BOOST_CHECK_EQUAL(block1.size(), 4);
    }
    {
        SaveBlock block2 {_reader};
        BOOST_CHECK_EQUAL(block2.size(), 8);
    }
}

struct NestedSaveBlockFixture {
    static constexpr std::array<unsigned char, 24> kTestSaveData
    {{
        0x0C, 0x00, 0x00, 0x00,
        0x08, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xFF,
        0x04, 0x00, 0x00, 0x00,
        0x02, 0x00, 0x00, 0x00,
    }};

    std::stringstream _truthBuff{std::ios_base::in | std::ios_base::out
                                 | std::ios_base::binary};
    Reader _reader{_truthBuff};

    NestedSaveBlockFixture() {
        auto data = reinterpret_cast<const char*>(kTestSaveData.data());
        _truthBuff.rdbuf()->sputn(data, sizeof(kTestSaveData));
    }
};

BOOST_FIXTURE_TEST_CASE(test_serialization_reads_nested_blocks,
                        NestedSaveBlockFixture) {
    uint32_t testValue {0xFFFFFFFF};
    {
        auto b1 = MakeSaveBlock(_reader);
        auto b1_1 = MakeSaveBlock(b1);
        b1_1.serialize<uint32_t>(testValue);
        BOOST_CHECK_EQUAL(testValue, 0x01);
    }
    {
        auto b2 = MakeSaveBlock(_reader);
        b2.serialize<uint32_t>(testValue);
        BOOST_CHECK_EQUAL(testValue, 0x02);
    }
}

struct TestSaveGameDataFixture {
    static constexpr std::array<unsigned char, 0xBC + 4> kTestSaveData
    {{
         0xBC, 0x00, 0x00, 0x00,
         't', 0x00, 'e', 0x00,
         's', 0x00, 't', 0x00,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
         0xFF, 0xFF, 0xFF, 0xFF,
     }};

    std::stringstream _truthBuff{std::ios_base::in | std::ios_base::out
                                 | std::ios_base::binary};
    SaveGameSerialization::Reader _reader{_truthBuff};

    TestSaveGameDataFixture() {
        _truthBuff.write(reinterpret_cast<const char*>(kTestSaveData.data()),
            sizeof(kTestSaveData));
    }
};

BOOST_FIXTURE_TEST_CASE(test_basic_state_serializes_with_reader,
                        TestSaveGameDataFixture) {
    BasicState basic {};
    SaveGameSerialization::SaveBlock block {_reader};
    bool result = SaveGameSerialization::serialize(block, basic);
    BOOST_CHECK(result);
    BOOST_CHECK_EQUAL(basic.saveName[0], 't');
    BOOST_CHECK_EQUAL(basic.saveName[1], 'e');
    BOOST_CHECK_EQUAL(basic.saveName[2], 's');
    BOOST_CHECK_EQUAL(basic.saveName[3], 't');
}

#if 0  // Disabled until we make a start on saving the game
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
#endif

BOOST_AUTO_TEST_SUITE_END()
