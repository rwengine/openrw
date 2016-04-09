#include <engine/SaveGame.hpp>
#include <engine/GameState.hpp>
#include <engine/GameWorld.hpp>
#include <objects/GameObject.hpp>
#include <engine/GameData.hpp>
#include <objects/VehicleObject.hpp>
#include <objects/CharacterObject.hpp>
#include <objects/InstanceObject.hpp>
#include <script/ScriptMachine.hpp>
#include <script/SCMFile.hpp>
#include <ai/PlayerController.hpp>
#include <items/WeaponItem.hpp>
#include <cstring>

// Original save game file data structures
typedef uint16_t BlockWord;
typedef uint32_t BlockDword;
typedef BlockDword BlockSize;

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
	SCMByte variables[16*4];
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
	BlockDword unknown0;
	BlockWord unknown1;
	BlockDword reference;
	StructPed info;
	BlockDword maxWantedLevel;
	BlockDword maxChaosLevel;
	uint8_t modelName[24];
	uint8_t align[2];
};

template<class T> void read(std::FILE* str, T& out) {
	std::fread(&out, sizeof(out), 1, str);
}

struct StructStoredCar {
	BlockDword modelId;
	glm::vec3 position;
	glm::vec3 rotation;
	BlockDword immunities;
	uint8_t colorFG;
	uint8_t colorBG;
	uint8_t radio;
	uint8_t variantA;
	uint8_t variantB;
	uint8_t bombType;
	uint8_t align0[2];
	
	// TODO Migrate to more available location (GameConstants?)
	enum /*VehicleImmunities*/ {
		Bulletproof = 1 << 0,
		Fireproof = 1 << 1,
		Explosionproof = 1 << 2,
		CollisionProof = 1 << 3,
		UnknownProof = 1 << 4
	};
	enum /*VehicleBombType*/ {
		NoBomb = 0,
		TimerBomb = 1,
		IgnitionBomb = 2,
		RemoteBomb = 3,
		TimerBombArmed = 4,
		IgnitionBombArmed = 5
	};
};

struct StructGarage {
	uint8_t type;
	uint8_t unknown0;
	uint8_t unknown1;
	uint8_t unknown2;
	uint8_t unknown3;
	uint8_t unknown4;
	uint8_t unknown5;
	uint8_t align0[2];
	BlockDword unknown6;
	BlockDword unknown7;
	uint8_t unknown8;
	uint8_t unknown9;
	uint8_t unknown10;
	uint8_t unknown11;
	uint8_t unknown12;
	uint8_t unknown13;
	uint8_t unknown14;
	uint8_t align1;
	float x1;
	float x2;
	float y1;
	float y2;
	float z1;
	float z2;
	float doorOpenStart;
	float doorOpenAngle;
	glm::vec2 unknownCoord1;
	glm::vec2 unknownCoord2;
	float doorAZ;
	float doorBZ;
	BlockDword unknown15;
	uint8_t unknown16;
	uint8_t align2[3];
	BlockDword unknown17;
	BlockDword unknown18;
	BlockDword unknown19;
	float unknown20;
	float unknown21;
	float unknown22;
	float unknown23;
	float unknown24;
	float unknown25;
	BlockDword unknown26;
	uint8_t unknown27;
	uint8_t unknown28;
	uint8_t unknown29;
	uint8_t unknown30;
	uint8_t unknown31;
	uint8_t unknown32;
	uint8_t align3[2];
};

struct Block2GarageData {
	BlockDword garageCount;
	BlockDword freeBombs;
	BlockDword freeResprays;
	BlockDword unknown0;
	BlockDword unknown1;
	BlockDword unknown2;
	BlockDword bfImportExportPortland;
	BlockDword bfImportExportShoreside;
	BlockDword bfImportExportUnused;
	BlockDword GA_21lastTime;
	StructStoredCar cars[18];
	
};

void SaveGame::writeGame(GameState& state, const std::string& file)
{
	std::FILE* saveFile = std::fopen(file.c_str(), "w");
	
	// BLOCK 0 - Variables
	BasicState block0Data = {};
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
		for(int i = 0; i < sizeof(Block0RunningScript::variables); i++) {
			script.variables[i] = thread.locals[i];
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
    return sz;
}

bool SaveGame::loadGame(GameState& state, const std::string& file)
{
	std::FILE* loadFile = std::fopen(file.c_str(), "r");
	
	// BLOCK 0
	BlockDword blockSize;
	fread(&blockSize, sizeof(BlockDword), 1, loadFile);
	
	BasicState block0Data;
	fread(&block0Data, sizeof(BasicState), 1, loadFile);
	
	BlockDword scriptBlockSize;
	fread(&scriptBlockSize, sizeof(BlockDword), 1, loadFile);

	char signature[4];
	fread(signature, sizeof(char), 4, loadFile);
	if( std::strncmp(signature, "SCR", 3) != 0 ) {
		return false;
	}

	fread(&scriptBlockSize, sizeof(BlockDword), 1, loadFile);

    BlockDword scriptVarCount;
    fread(&scriptVarCount, sizeof(BlockDword), 1, loadFile);
    assert(scriptVarCount == state.script->getFile()->getGlobalsSize());

    fread(state.script->getGlobals(), sizeof(SCMByte), scriptVarCount, loadFile);

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
    for(unsigned int p = 0; p < playerCount; ++p) {
		read(loadFile, players[p].unknown0);
		read(loadFile, players[p].unknown1);
		read(loadFile, players[p].reference);
		read(loadFile, players[p].info);
		read(loadFile, players[p].maxWantedLevel);
		read(loadFile, players[p].maxChaosLevel);
		read(loadFile, players[p].modelName);
		read(loadFile, players[p].align);
	}

	// BLOCK 2
	BlockDword garageBlockSize = readDword(loadFile);
	BlockDword garageDataSize = readDword(loadFile);

	Block2GarageData garageData;
	fread(&garageData, sizeof(Block2GarageData), 1, loadFile);

	StructGarage garages[garageData.garageCount];
	fread(garages, sizeof(StructGarage), garageData.garageCount, loadFile);

	// Insert Game State.
	state.hour = block0Data.gameHour;
	state.minute = block0Data.gameMinute;
	state.gameTime = block0Data.timeMS / 1000.f;
	state.currentWeather = block0Data.nextWeather;
    state.cameraPosition = block0Data.cameraPosition;

	state.scriptOnMissionFlag = (unsigned int*)state.script->getGlobals() + (size_t)scriptData.onMissionOffset;

	auto& threads = state.script->getThreads();
	for(int s = 0; s < numScripts; ++s) {
		state.script->startThread(scripts[s].programCounter);
		SCMThread& thread = threads.back();
		// thread.baseAddress // ??
        strncpy(thread.name, scripts[s].name, sizeof(SCMThread::name)-1);
		thread.conditionResult = scripts[s].ifFlag;
		thread.conditionCount = scripts[s].ifNumber;
		thread.stackDepth = scripts[s].stackCounter;
		for(int i = 0; i < SCM_STACK_DEPTH; ++i) {
			thread.calls[i] = scripts[s].stack[i];
		}
		 /* TODO not hardcode +33 ms */
		thread.wakeCounter = scripts[s].wakeTimer - block0Data.lastTick + 33;
		for(int i = 0; i < sizeof(Block0RunningScript::variables); ++i) {
			thread.locals[i] = scripts[s].variables[i];
		}
	}

	if( playerCount > 0 ) {
		auto& ply = players[0];
		std::cout << ply.reference << std::endl;
		auto player = state.world->createPlayer(players[0].info.position);
		player->mHealth = players[0].info.health;
		state.playerObject = player->getGameObjectID();
		state.maxWantedLevel = players[0].maxWantedLevel;
		for(int w = 0; w < 13; ++w) {
			auto& wep = ply.info.weapons[w];
			if(wep.weaponId != 0) {
				auto& weaponInfo = state.world->data->weaponData.at(wep.weaponId);
				auto item = new WeaponItem(player, weaponInfo);
				player->addToInventory(item);
			}
		}
	}

	// TODO restore garage data
	// http://gtaforums.com/topic/758692-gta-iii-save-file-documentation/
	for(int g = 0; g < garageData.garageCount; ++g) {
		auto& garage = garages[g];
		state.garages.push_back({
			glm::vec3(garage.x1, garage.y1, garage.z1),
			glm::vec3(garage.x2, garage.y2, garage.z2),
			garage.type
		});
		auto& gameGarage = state.garages.back();
		auto center = (gameGarage.min + gameGarage.max)/2.f;
		// Find the nearest dynamic instance?
		float distance = std::numeric_limits<float>::max();
		GameObject* nearinst = nullptr;
		for(std::pair<GameObjectID, GameObject*> object : state.world->instancePool.objects) {
			auto instance = static_cast<InstanceObject*>(object.second);
			if( instance->dynamics ) {
				float idist = glm::distance(center, instance->getPosition());
				if( idist < distance ) {
					distance = idist;
					nearinst = instance;
				}
			}
		}
		// Nearinst is probably the garage door.
	}
	for(int c = 0; c < 18; ++c) {
		if(garageData.cars[c].modelId == 0) continue;
		auto& car = garageData.cars[c];
		glm::quat rotation(-glm::vec3(car.rotation.z, car.rotation.y, car.rotation.x));
		
		VehicleObject* vehicle = state.world->createVehicle(car.modelId, car.position, rotation);
		vehicle->setPrimaryColour(car.colorFG);
		vehicle->setSecondaryColour(car.colorBG);
	}

	std::fclose(loadFile);
	
	return true;
}

#include <iconv.h>
#include <dirent.h>
bool SaveGame::getSaveInfo(const std::string& file, BasicState *basicState)
{
	std::FILE* loadFile = std::fopen(file.c_str(), "r");

	SaveGameInfo info;
	info.savePath = file;
	
	// BLOCK 0
	BlockDword blockSize;
	if( fread(&blockSize, sizeof(BlockDword), 1, loadFile) == 0 ) {
		return false;
	}
	
	// Read block 0 into state
	if( fread(basicState, sizeof(BasicState), 1, loadFile) == 0 ) {
		return false;
	}
	
	std::fclose(loadFile);
	size_t bytes = 0;
	for(;; bytes++ ) {
		if(basicState->saveName[bytes-1] == 0 && basicState->saveName[bytes] == 0) break;
	}
	size_t outSize = 24;
	char outBuff[48];
	char* outCur = outBuff;
	auto icv = iconv_open("UTF-8", "UTF-16");
	char* saveName = (char*)basicState->saveName;

	// Convert to UTF-8 and copy back to the return struct
	iconv(icv, &saveName, &bytes, &outCur, &outSize);
	strcpy(basicState->saveName, outBuff);

	return true;
}

std::vector< SaveGameInfo > SaveGame::getAllSaveGameInfo()
{
	// TODO consider windows
	auto homedir = getenv("HOME");
	if( homedir == nullptr ) {
		std::cerr << "Unable to determine home directory" << std::endl;
		return {};
	}
	const char gameDir[] = "GTA3 User Files";
	std::string gamePath(homedir);
	gamePath.append("/");
	gamePath.append(gameDir);

	DIR* dp = opendir(gamePath.c_str());
	dirent* ep;
	std::string realName;
	if ( dp == NULL ) {
		return {};
	}
	std::vector<SaveGameInfo> infos;
	while( (ep = readdir(dp)) )
	{
		if ( ep->d_type == DT_REG ) {
			realName = ep->d_name;
			if(realName.find(".b") != realName.npos) {
				std::string path = gamePath+"/"+realName;
				infos.emplace_back(SaveGameInfo{path, false, {}});
				infos.back().valid = getSaveInfo(infos.back().savePath, &infos.back().basicState);
			}
		}
	}
	closedir(dp);
	return infos;
}

