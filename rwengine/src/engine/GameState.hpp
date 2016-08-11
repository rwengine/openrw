#pragma once
#ifndef _GAMESTATE_HPP_
#define _GAMESTATE_HPP_
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <string>
#include <bitset>
#include <map>
#include <vector>
#include <objects/ObjectTypes.hpp>
#include <engine/ScreenText.hpp>
#include <data/VehicleGenerator.hpp>
#include <engine/GameInputState.hpp>

class GameWorld;
class GameObject;
class ScriptMachine;
class PlayerController;
struct CutsceneData;

struct SystemTime
{
	uint16_t year;
	uint16_t month;
	uint16_t dayOfWeek;
	uint16_t day;
	uint16_t hour;
	uint16_t minute;
	uint16_t second;
	uint16_t millisecond;
};

/** Block 0 State */
struct BasicState
{
	/// /!\ This is wchar_t[24] in the original format /!\ we convert on load for convenience
	char saveName[48];
	SystemTime saveTime;
	uint32_t unknown;
	uint16_t islandNumber;
	glm::vec3 cameraPosition;
	uint32_t gameMinuteMS;
	uint32_t lastTick;
	uint8_t gameHour;
	uint8_t _align0[3];
	uint8_t gameMinute;
	uint8_t _align1[3];
	uint16_t padMode;
	uint8_t _align2[2];
	uint32_t timeMS;
	float timeScale;
	float timeStep;
	float timeStep_unclipped; // Unknown purpose
	uint32_t frameCounter;
	float timeStep2;
	float framesPerUpdate;
	float timeScale2;
	uint16_t lastWeather;
	uint8_t _align3[2];
	uint16_t nextWeather;
	uint8_t _align4[2];
	uint16_t forcedWeather;
	uint8_t _align5[2];
	float weatherInterpolation;
	uint8_t dateTime[24]; // Unused
	uint32_t weatherType;
	float cameraData;
	float cameraData2;

	BasicState ();
};

/** Block 15 player info */
struct PlayerInfo
{
	uint16_t money;
	uint8_t unknown1;
	uint16_t unknown2;
	uint8_t unknown3;
	float unknown4;
	uint16_t displayedMoney;
	uint16_t hiddenPackagesCollected;
	uint16_t hiddenPackageCount;
	uint8_t neverTired;
	uint8_t fastReload;
	uint8_t thaneOfLibertyCity;
	uint8_t singlePayerHealthcare;
	uint8_t unknown5[70];

	PlayerInfo ();
};

/** Block 17 */
struct GameStats
{
	uint16_t playerKills;
	uint16_t otherKills;
	uint16_t carsExploded;
	uint16_t shotsHit;
	uint16_t pedTypesKilled[23];
	uint16_t helicoptersDestroyed;
	uint16_t playerProgress;
	uint16_t explosiveKgsUsed;
	uint16_t bulletsFired;
	uint16_t bulletsHit;
	uint16_t carsCrushed;
	uint16_t headshots;
	uint16_t timesBusted;
	uint16_t timesHospital;
	uint16_t daysPassed;
	uint16_t mmRainfall;
	uint16_t insaneJumpMaxDistance;
	uint16_t insaneJumpMaxHeight;
	uint16_t insaneJumpMaxFlips;
	uint16_t insangeJumpMaxRotation;
	/*
	 * 0 none completed
	 * 1 insane stunt
	 * 2 perfect insane stunt
	 * 3 double insane stunt
	 * 4 perfect double insane stunt
	 * 5 triple insane stunt
	 * 6 perfect " " "
	 * 7 quadruple
	 * 8 perfect quadruple
	 */
	uint16_t bestStunt;
	uint16_t uniqueStuntsFound;
	uint16_t uniqueStuntsTotal;
	uint16_t missionAttempts;
	uint16_t missionsPassed;
	uint16_t passengersDroppedOff;
	uint16_t taxiRevenue;
	uint16_t portlandPassed;
	uint16_t stauntonPassed;
	uint16_t shoresidePassed;
	uint16_t bestTurismoTime;
	float distanceWalked;
	float distanceDriven;
	uint16_t patriotPlaygroundTime;
	uint16_t aRideInTheParkTime;
	uint16_t grippedTime;
	uint16_t multistoryMayhemTime;
	uint16_t peopleSaved;
	uint16_t criminalsKilled;
	uint16_t highestParamedicLevel;
	uint16_t firesExtinguished;
	uint16_t longestDodoFlight;
	uint16_t bombDefusalTime;
	uint16_t rampagesPassed;
	uint16_t totalRampages;
	uint16_t totalMissions;
	uint16_t fastestTime[16]; // not used
	uint16_t highestScore[16];
	uint16_t peopleKilledSinceCheckpoint; // ?
	uint16_t peopleKilledSinceLastBustedOrWasted;
	char lastMissionGXT[8];

	GameStats ();
};

struct TextDisplayData
{
	// This is set by the final display text command.
	std::string text;
	glm::vec2 position;

	glm::vec4 colourFG;
	glm::vec4 colourBG;
};

/**
 * Data about a blip
 */
struct BlipData
{
	int id;
	
	enum BlipType
	{
		Location = 0,
		Vehicle = 1,
		Pickup = 2,
		Character = 3,
	};
	BlipType type;
	GameObjectID target;
	// If target is null then use coord
	glm::vec3 coord;
	
	/* Texture for use in the radar */
	std::string texture;
	
	enum DisplayMode
	{
		Hide = 0,
		RadarOnly = 1,
		Show = 2
	};
	
	/* Should the blip be displayed? */
	DisplayMode display;
	
	BlipData()
	: id(-1), type(Location), target(0), display(Show)
	{ }

	int getScriptObjectID() const
	{
		return id;
	}
};

/**
 * Data for garages
 */
struct GarageInfo
{
	enum /*GarageType*/ {
		GARAGE_MISSION = 1,
		GARAGE_BOMBSHOP1 = 2,
		GARAGE_BOMBSHOP2 = 3,
		GARAGE_BOMBSHOP3 = 4,
		GARAGE_RESPRAY = 5,
		GARAGE_INVALID = 6,
		GARAGE_SPECIFIC_CARS_ONLY = 7, /* See Opcode 0x21B */
		GARAGE_COLLECTCARS1 = 8, /* See Opcode 0x03D4 */
		GARAGE_COLLECTCARS2 = 9,
		GARAGE_COLLECTCARS3 = 10, /* Unused */
		GARAGE_OPENFOREXIT = 11,
		GARAGE_INVALID2  = 12,
		GARAGE_CRUSHER = 13,
		GARAGE_MISSION_KEEPCAR = 14,
		GARAGE_FOR_SCRIPT = 15,
		GARAGE_HIDEOUT_ONE = 16, /* Portland */
		GARAGE_HIDEOUT_TWO = 17, /* Staunton */
		GARAGE_HIDEOUT_THREE = 18, /* Shoreside */
		GARAGE_FOR_SCRIPT2 = 19,
		GARAGE_OPENS_FOR_SPECIFIC_CAR = 20,
		GARAGE_OPENS_ONCE = 21
	};
	int id;
	glm::vec3 min;
	glm::vec3 max;
	int type;

	GarageInfo(int id_,
			   const glm::vec3 min_,
			   const glm::vec3 max_,
			   int type_)
		: id(id_)
		, min(min_)
		, max(max_)
		, type(type_)
	{ }

	int getScriptObjectID() const
	{
		return id;
	}
};

/**
 * Gameplay state object that holds persistent state, and references runtime
 * world state.
 */
struct GameState
{
	/**
	  Basic Game State
	 */
	BasicState basic;

	/**
	  Player stats
	 */
	PlayerInfo playerInfo;

	/**
	  Game Stats
	  */
	GameStats gameStats;

	/**
	 * Second since game was started
	 */
	float gameTime;
	unsigned int currentProgress;
	unsigned int maxProgress;

	unsigned int maxWantedLevel;

	GameObjectID playerObject;

	/**
	 * @brief Stores a pointer to script global that stores the on-mission state.
	 */
	int32_t* scriptOnMissionFlag;
	
	/** Objects created by the current mission */
	std::vector<GameObject*> missionObjects;
	
	bool overrideNextStart;
	glm::vec4 nextRestartLocation;

	bool fadeOut;
	float fadeStart;
	float fadeTime;
	bool fadeSound;
	glm::u16vec3 fadeColour;

	std::string currentSplash;

	bool skipCutscene;
	bool isIntroPlaying;
	CutsceneData* currentCutscene;
	float cutsceneStartTime;
	/** Flag for rendering cutscene letterbox */
	bool isCinematic;
	
	std::string lastMissionName;
	
	/// Stores the "special" character and cutscene model indices.
	std::map<unsigned short, std::string> specialCharacters;
	std::map<unsigned short, std::string> specialModels;

	/// Handles on screen text behaviour
	ScreenText text;

	TextDisplayData nextText;
	/**
	 * Stores temporary, one-tick messages
	 */
	std::vector<TextDisplayData> texts;

	/** The camera near value currently set by the script */
	float cameraNear;
	bool cameraFixed;
	glm::vec3 cameraPosition;
	glm::quat cameraRotation;

	GameObjectID cameraTarget;
	
	std::vector<VehicleGenerator> vehicleGenerators;
	
	std::map<int, BlipData> radarBlips;

	std::vector<GarageInfo> garages;

	/**
	 * Bitsets for the car import / export list mission
	 */
	std::bitset<32> importExportPortland;
	std::bitset<32> importExportShoreside;
	std::bitset<32> importExportUnused;

	/**
	 * State of the game input
	 */
	GameInputState input;

	/**
	 * World to use for this state, this isn't saved, just used at runtime
	 */
	GameWorld* world;

	/**
	 * Script Machine associated with this state if it exists.
	 */
	ScriptMachine* script;

	GameState();

	/**
	 * Adds a blip to the state, returning it's ID.
	 */
	int addRadarBlip(BlipData& blip);
	
	/**
	 * Removes a blip
	 */
	void removeBlip(int blip);
};

#endif
