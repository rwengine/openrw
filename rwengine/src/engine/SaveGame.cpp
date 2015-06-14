#include <engine/SaveGame.hpp>
#include <engine/GameState.hpp>
#include <engine/GameWorld.hpp>
#include <objects/GameObject.hpp>
#include <objects/VehicleObject.hpp>
#include <objects/CharacterObject.hpp>
#include <script/ScriptMachine.hpp>
#include <script/SCMFile.hpp>
#include <ai/PlayerController.hpp>

#include <fstream>
#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/map.hpp>

namespace cereal
{

template<class Archive>
void serialize(Archive& archive, 
          glm::vec3& s)
{ 
	archive(s.x, s.y, s.z);
}

template<class Archive>
void serialize(Archive& archive, 
          glm::vec4& s)
{ 
	archive(s.x, s.y, s.z, s.w);
}

template<class Archive>
void serialize(Archive& archive, 
          glm::u16vec3& s)
{ 
	archive(s.x, s.y, s.z);
}

template<class Archive>
void serialize(Archive& archive, 
          glm::quat& s)
{ 
	archive(s.x, s.y, s.z, s.w);
}

template<class Archive>
void serialize(Archive& archive, 
          OnscreenText& t)
{ 
	archive(
		t.id,
		t.osTextString,
		t.osTextStart,
		t.osTextTime,
		t.osTextStyle);
}

template<class Archive>
void serialize(Archive& archive, 
          VehicleGenerator& s)
{ 
	archive(
		s.position,
		s.heading,
		s.vehicleID,
		s.colourFG,
		s.colourBG,
		s.alwaysSpawn,
		s.alarmThreshold,
		s.lockedThreshold,
		s.minDelay,
		s.maxDelay,
		s.lastSpawnTime,
		s.remainingSpawns);
}

template<class Archive>
void serialize(Archive& archive, 
          BlipData& s)
{ 
	archive(
		s.id,
		s.target,
		s.coord,
		s.texture,
		s.display);
}

template<class Archive>
void serialize(Archive& archive, 
          GarageInfo& s)
{ 
	archive(
		s.min,
		s.max,
		s.type);
}

template<class Archive>
void serialize(Archive& archive, 
          GameState& s)
{ 
	archive(
		s.gameTime,
		s.currentProgress,
		s.maxProgress,
		s.numMissions,
		s.numHiddenPackages,
		s.numHiddenPackagesDiscovered,
		s.numUniqueJumps,
		s.numRampages,
		s.maxWantedLevel,
		s.playerObject,
		s.currentWeather,
		s.missionObjects,
		s.overrideNextStart,
		s.nextRestartLocation,
		s.fadeOut,
		s.fadeStart,
		s.fadeTime,
		s.fadeSound,
		s.fadeColour,
		s.currentSplash,
		s.skipCutscene,
		s.isIntroPlaying,
		s.isCinematic,
		s.hour,
		s.minute,
		s.lastMissionName,
		s.specialCharacters,
		s.specialModels,
		s.text,
		s.cameraNear,
		s.cameraFixed,
		s.cameraPosition,
		s.cameraRotation,
		s.cameraTarget,
		s.vehicleGenerators,
		s.radarBlips); 
}

template<class Archive>
void serialize(Archive& archive, 
          SCMThread& s)
{ 
	archive(
		s.name,
		s.baseAddress,
		s.programCounter,
		s.conditionCount,
		s.conditionResult,
		s.conditionMask,
		s.conditionAND,
		s.wakeCounter,
		s.locals,
		s.isMission,
		s.finished,
		s.stackDepth,
		s.calls);
}

template<class Archive>
void serialize(Archive& archive, 
          ScriptMachine& s)
{ 
	archive(
		s.getThreads(),
		s.getGlobalData());
}
}

void SaveGame::writeState(GameState& state, const std::string& file)
{
	std::ofstream os(file.c_str());

	{
		cereal::JSONOutputArchive oa(os);

		oa(state);
	}
}

bool SaveGame::loadState(GameState& state, const std::string& file)
{
	std::ifstream is(file.c_str());

	{
		cereal::JSONInputArchive ia(is);

		ia(state);
	}

	return true;
}

void SaveGame::writeScript(ScriptMachine& sm, const std::string& file)
{
	std::ofstream os(file.c_str());

	{
		cereal::JSONOutputArchive oa(os);

		oa(sm);
	}
}

bool SaveGame::loadScript(ScriptMachine& sm, const std::string& file)
{
	std::ifstream is(file.c_str());

	{
		cereal::JSONInputArchive ia(is);

		ia(sm);
	}

	return true;
}

void SaveGame::writeObjects(GameWorld& world, const std::string& file)
{
	std::ofstream os(file.c_str());

	{
		cereal::JSONOutputArchive oa(os);

		std::vector<GameObject*> writeable;
		for( auto& p : world.objects )
		{
			switch( p.second->type() )
			{
				case GameObject::Vehicle:
				case GameObject::Character:
					break;
				default:
					continue;
			}
			if( p.second->getLifetime() == GameObject::TrafficLifetime )
			{
				continue;
			}
			writeable.push_back(p.second);
		}

		// Write object count.
		oa(writeable.size());
		for( GameObject* saved : writeable )
		{
			oa(saved->getGameObjectID());
			oa(saved->type());
			oa(saved->getLifetime());
			oa(saved->getPosition());
			oa(saved->getRotation());
			switch( saved->type() )
			{
			case GameObject::Vehicle:
			{
				auto vehicle = static_cast<VehicleObject*>(saved);
				oa(vehicle->vehicle->ID);
			} break;
			case GameObject::Character:
			{
				auto character = static_cast<CharacterObject*>(saved);
				oa(character->ped->ID);
			} break;
			}
		}
	}
}


bool SaveGame::loadObjects(GameWorld& world, const std::string& file)
{
	std::ifstream is(file.c_str());

	{
		cereal::JSONInputArchive ia(is);

		std::size_t num;
		ia(num);
		for(int i = 0; i < num; i++)
		{
			GameObjectID gameID;
			GameObject::Type type;
			GameObject::ObjectLifetime lifetime;
			glm::vec3 translation;
			glm::quat orientation;

			ia(gameID);
			ia(type);
			ia(lifetime);
			ia(translation);
			ia(orientation);

			switch( type )
			{
			case GameObject::Vehicle:
			{
				ObjectID id;
				ia(id);
				auto vehicle = world.createVehicle(id, translation, orientation, gameID);
				vehicle->setLifetime(lifetime);
			} break;
			case GameObject::Character:
			{
				ObjectID id;
				ia(id);
				CharacterObject* character;
				if( lifetime == GameObject::PlayerLifetime )
				{
					character = world.createPlayer(translation, orientation, gameID);
				}
				else
				{
					character = world.createPedestrian(id, translation, orientation, gameID);
				}
			} break;
			}
		}
	}

	return true;
}

// Original save game file data structures
typedef uint16_t BlockWord;
typedef uint32_t BlockDword;
typedef BlockDword BlockSize;

struct Block0Data {
	uint16_t saveName[24];
	BlockWord year;
	BlockWord month;
	BlockWord weekday;
	BlockWord day;
	BlockWord hour;
	BlockWord minute;
	BlockWord second;
	BlockWord milliseconds;
	BlockDword unknown;
	BlockDword islandNumber;
	glm::vec3 cameraPosition;
	BlockDword gameMinuteMS;
	BlockDword lastTick;
	uint8_t gameHour;
	uint8_t _align0[3];
	uint8_t gameMinute;
	uint8_t _align1[3];
	BlockWord padMode;
	uint8_t _align2[2];
	BlockDword timeMS;
	float timeScale;
	float timeStep;
	float timeStep_unclipped; // Unknown purpose
	BlockDword frameCounter;
	float timeStep2;
	float framesPerUpdate;
	float timeScale2;
	BlockWord lastWeather;
	uint8_t _align3[2];
	BlockWord nextWeather;
	uint8_t _align4[2];
	BlockWord forcedWeather;
	uint8_t _align5[2];
	float weatherInterpolation;
	uint8_t dateTime[24]; // Unused
	BlockDword weatherType;
	float cameraData;
	float cameraData2;
};

struct Block0ContactInfo {
	BlockDword missionFlag;
	BlockDword baseBrief;
};

struct Block0BuildingSwap {
	BlockDword type;
	BlockDword handle;
	BlockDword newModel;
	BlockDword oldModel;
};

struct Block0InvisibilitySettings {
	BlockDword type;
	BlockDword handle;
};

struct Block0RunningScript {
	uint32_t nextPointer;
	uint32_t prevPointer;
	char name[8];
	BlockDword programCounter;
	BlockDword stack[4];
	BlockDword unknown0;
	BlockDword unknown1;
	BlockWord stackCounter;
	BlockWord unknown2;
	BlockDword scriptVariables[16];
	BlockDword timerA;
	BlockDword timerB;
	uint8_t ifFlag;
	uint8_t unknown3;
	uint8_t unknown4;
	uint8_t _align0;
	BlockDword wakeTimer;
	BlockWord ifNumber; // ?
	uint8_t unknown[6];
};

struct Block0ScriptData {
	BlockDword onMissionOffset;
	Block0ContactInfo contactInfo[16];
	uint8_t unknown[0x100];
	BlockDword lastMissionPassedTime;
	Block0BuildingSwap buildingSwap[25];
	Block0InvisibilitySettings invisibilitySettings[20];
	uint8_t scriptRunning;
	uint8_t _align0[3];
	BlockDword mainSize;
	BlockDword largestMissionSize;
	BlockWord missionCount;
	uint8_t _align1[2];
};

struct StructWeaponSlot {
	BlockDword weaponId;
	BlockDword unknown0;
	BlockDword inClip;
	BlockDword totalBullets;
	BlockDword unknown1;
	BlockDword unknown2;
};
struct StructPed {
	uint8_t unknown0_[52];
	glm::vec3 position;
	uint8_t unknown1[640];
	float health;
	float armour;
	uint8_t unknown2[148];
	StructWeaponSlot weapons[13];
	uint8_t unknown3[348];
};

// NOTE commented members are read manually, due to alignment.
struct Block1PlayerPed {
	//BlockDword unknown0;
	//BlockWord unknown1;
	alignas(uint8_t) BlockDword reference; // 0x0A
	alignas(uint8_t) StructPed info; // 0x0C
	BlockDword maxWantedLevel;
	BlockDword maxChaosLevel;
	uint8_t modelName[24];
	uint8_t align[2];
};

void SaveGame::writeGame(GameState& state, const std::string& file)
{
	std::FILE* saveFile = std::fopen(file.c_str(), "w");
	
	// BLOCK 0 - Variables
	Block0Data block0Data = {};
	//strcpy(block0Data.saveName, "OpenRW Save Game");
	block0Data.islandNumber = 1;
	block0Data.cameraPosition = glm::vec3(0.f);
	block0Data.gameMinuteMS = 1000;
	block0Data.lastTick = 1;
	block0Data.gameHour = 12;
	block0Data.gameMinute = 13;
	block0Data.padMode = 1;
	block0Data.timeMS = 10000;
	block0Data.timeScale = 1.0;
	block0Data.timeStep = 1.0/60.f;
	block0Data.timeStep_unclipped = block0Data.timeStep;
	block0Data.frameCounter = 1000;
	block0Data.timeStep2 = 1.0;
	block0Data.framesPerUpdate = 1.0;
	block0Data.timeScale2 = 1.0;
	block0Data.lastWeather = 1;
	block0Data.nextWeather = 1;
	block0Data.weatherInterpolation = 1.0;
	block0Data.weatherType = 1;
	
	BlockSize block0Size = sizeof(block0Data); // TODO calculate correctly.
	fwrite(&block0Size, sizeof(BlockSize), 1, saveFile);
	fwrite(&block0Data, sizeof(block0Data), 1, saveFile);
	
	// BLOCK 0 - 0 Script 
	const char header[4] = "SCR";
	BlockSize block0ScriptSize = sizeof(Block0ScriptData);
	BlockSize block0ScriptHeaderSize = block0ScriptSize + sizeof(char) * 4 + sizeof(BlockDword);
	fwrite(header, sizeof(char), 4, saveFile);
	fwrite(&block0ScriptHeaderSize, sizeof(BlockSize), 1, saveFile);
	BlockDword scriptVariablesCount = state.script->getGlobalData().size();
	fwrite(&scriptVariablesCount, sizeof(BlockDword), 1, saveFile);
	fwrite(state.script->getGlobals(), sizeof(SCMByte), scriptVariablesCount, saveFile);
	
	BlockDword scriptDataSize = 0x03C8;
	fwrite(&scriptDataSize, sizeof(BlockDword), 1, saveFile);

	Block0ScriptData block0ScriptData = {};
	block0ScriptData.onMissionOffset = (BlockDword)((SCMByte*)state.scriptOnMissionFlag - state.script->getGlobals());
	block0ScriptData.lastMissionPassedTime = 0;
	block0ScriptData.scriptRunning = 1;
	block0ScriptData.mainSize = state.script->getFile()->getMainSize();
	block0ScriptData.largestMissionSize = state.script->getFile()->getLargestMissionSize();
	block0ScriptData.missionCount = state.script->getFile()->getMissionOffsets().size();
	fwrite(&block0ScriptData, sizeof(block0ScriptData), 1, saveFile);

	BlockDword scriptCount = state.script->getThreads().size();
	fwrite(&scriptCount, sizeof(BlockDword), 1, saveFile);

	for(SCMThread& thread : state.script->getThreads())
	{
		Block0RunningScript script = {};
		strcpy(script.name, thread.name);
		script.programCounter = thread.programCounter;
		for(int i = 0; i < SCM_STACK_DEPTH; i++) {
			script.stack[i] = thread.calls[i];
		}
		script.stackCounter = thread.stackDepth;
		for(int i = 0; i < 16; i++) {
			script.scriptVariables[i] = *(((BlockDword*)thread.locals.data())+i);
		}
		script.timerA = *(BlockDword*)(thread.locals.data() + 16 * sizeof ( SCMByte ) * 4);
		script.timerB = *(BlockDword*)(thread.locals.data() + 16 * sizeof ( SCMByte ) * 4);
		script.ifFlag = thread.conditionResult;
		script.wakeTimer = thread.wakeCounter;
		script.ifNumber = thread.conditionCount;
		fwrite(&script, sizeof(block0ScriptData), 1, saveFile);
	}
}

const size_type DWORDSZ = sizeof(BlockDword);
BlockDword readDword(std::FILE* file)
{
	BlockDword sz;
	fread(&sz, sizeof(BlockDword), 1, file);
}

bool SaveGame::loadGame(GameState& state, const std::string& file)
{
	std::FILE* loadFile = std::fopen(file.c_str(), "r");
	
	// BLOCK 0
	BlockDword blockSize;
	fread(&blockSize, sizeof(BlockDword), 1, loadFile);
	
	Block0Data block0Data;
	fread(&block0Data, sizeof(block0Data), 1, loadFile);
	
	BlockDword scriptBlockSize;
	fread(&scriptBlockSize, sizeof(BlockDword), 1, loadFile);

	char signature[4];
	fread(signature, sizeof(char), 4, loadFile);
	if( std::strncmp(signature, "SCR", 3) != 0 ) {
		return false;
	}

	fread(&scriptBlockSize, sizeof(BlockDword), 1, loadFile);

	BlockDword scriptVars;
	fread(&scriptVars, sizeof(BlockDword), 1, loadFile);

	SCMByte bytes[scriptVars];
	fread(bytes, sizeof(SCMByte), scriptVars, loadFile);

	BlockDword scriptDataBlockSize;
	fread(&scriptDataBlockSize, sizeof(BlockDword), 1, loadFile);
	if( scriptDataBlockSize != 0x03C8 ) {
		return false;
	}
	
	Block0ScriptData scriptData;
	fread(&scriptData, sizeof(Block0ScriptData), 1, loadFile);
	
	BlockDword numScripts;
	fread(&numScripts, DWORDSZ, 1, loadFile);
	Block0RunningScript scripts[numScripts];
	fread(scripts, sizeof(Block0RunningScript), numScripts, loadFile);

	// BLOCK 1
	BlockDword playerBlockSize = readDword(loadFile);
	BlockDword playerInfoSize = readDword(loadFile);
	
	BlockDword playerCount = readDword(loadFile);
	Block1PlayerPed players[playerCount];
	BlockDword unknownPlayerValue = readDword(loadFile);
	BlockWord unknownPlayerValue2;
	fread(&unknownPlayerValue2, sizeof(BlockWord), 1, loadFile);
	fread(players, sizeof(Block1PlayerPed), playerCount, loadFile);

	// Insert Game State.
	state.hour = block0Data.gameHour;
	state.minute = block0Data.gameMinute;
	state.gameTime = block0Data.timeMS / 1000.f;
	state.currentWeather = block0Data.nextWeather;
	state.cameraPosition = block0Data.cameraPosition;
	
	for(int v = 0; v < scriptVars; ++v) {
		state.script->getGlobals()[v] = bytes[v];
	}

	state.scriptOnMissionFlag = (unsigned int*)state.script->getGlobals() + (size_t)scriptData.onMissionOffset;

	auto& threads = state.script->getThreads();
	for(int s = 0; s < numScripts; ++s) {
		state.script->startThread(scripts[s].programCounter);
		SCMThread& thread = threads.back();
		// thread.baseAddress // ??
		thread.conditionResult = scripts[s].ifFlag;
		thread.conditionCount = scripts[s].ifNumber;
		thread.stackDepth = scripts[s].stackCounter;
		for(int i = 0; i < SCM_STACK_DEPTH; ++i) {
			thread.calls[i] = scripts[s].stack[i];
		}
		thread.wakeCounter = scripts[s].wakeTimer;
	}

	if( playerCount > 0 ) {
		auto player = state.world->createPlayer(players[0].info.position);
		player->mHealth = players[0].info.health;
		state.playerObject = player->getGameObjectID();
		state.maxWantedLevel = players[0].maxWantedLevel;
	}
	
	return true;
}

