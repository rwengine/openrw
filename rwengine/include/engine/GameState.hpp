#pragma once
#ifndef _GAMESTATE_HPP_
#define _GAMESTATE_HPP_
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <string>
#include <map>
#include <vector>
#include <objects/ObjectTypes.hpp>

class GameWorld;
class GameObject;
class ScriptMachine;
class PlayerController;
struct CutsceneData;

struct TextDisplayData
{
	// This is set by the final display text command.
	std::string text;
	glm::vec2 position;

	glm::vec4 colourFG;
	glm::vec4 colourBG;
};

struct OnscreenText
{
	std::string id;
	std::string osTextString;
	float osTextStart;
	float osTextTime;
	unsigned short osTextStyle;
	std::string osTextVar;
	
	enum /*TextStyle*/
	{
		/// Used for subtitles
		HighPriority = 0,
		/// Mission completed message
		CenterBig = 1,
		/// Right aligned mission names
		MissionName = 2,
		/// Help text (top left, black background)
		Help = 12
	};
};

/**
 * Stores information about where the game can generate vehicles.
 */
struct VehicleGenerator
{
	glm::vec3 position;
	float heading;
	/** ID of the vehicle to spawn, or -1 for random. */
	int vehicleID;
	int colourFG;
	int colourBG;
	bool alwaysSpawn;
	short alarmThreshold;
	short lockedThreshold;
	
	int minDelay;
	int maxDelay;
	/** Incrementing timer (in ms), will only spawn a vehicle when minDelay < spawnTimer and will always spawn if maxDelay < spawnTimer */
	int lastSpawnTime;
	
	/** Number of vehicles left to spawn 0-100, 101 = never decrement. */
	int remainingSpawns;
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
		GARAGE_CRUSHER = 13, /* Unused */
		GARAGE_MISSION_KEEPCAR = 14,
		GARAGE_FOR_SCRIPT = 15,
		GARAGE_HIDEOUT_ONE = 16, /* Portland */
		GARAGE_HIDEOUT_TWO = 17, /* Staunton */
		GARAGE_HIDEOUT_THREE = 18, /* Shoreside */
		GARAGE_FOR_SCRIPT2 = 19,
		GARAGE_OPENS_FOR_SPECIFIC_CAR = 20,
		GARAGE_OPENS_ONCE = 21
	};
	glm::vec3 min;
	glm::vec3 max;
	int type;
};

/**
 * Gameplay state object that holds persistent state, and references runtime
 * world state.
 */
struct GameState
{
	/**
	 * Second since game was started
	 */
	float gameTime;
	unsigned int currentProgress;
	unsigned int maxProgress;
	unsigned int numMissions;
	unsigned int numHiddenPackages;
	unsigned int numHiddenPackagesDiscovered;
	unsigned int numUniqueJumps;
	unsigned int numRampages;
	unsigned int maxWantedLevel;

	GameObjectID playerObject;

	unsigned int currentWeather;

	/**
	 * @brief Stores a pointer to script global that stores the on-mission state.
	 */
	unsigned int *scriptOnMissionFlag;
	
	/** Objects created by the current mission */
	std::vector<GameObjectID> missionObjects;
	
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

	short hour;
	short minute;
	
	std::string lastMissionName;
	
	/// Stores the "special" character and cutscene model indices.
	std::map<unsigned short, std::string> specialCharacters;
	std::map<unsigned short, std::string> specialModels;

	/**
	 * Stores long-lasting on screen messages
	 */
	std::vector<OnscreenText> text;
	
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
