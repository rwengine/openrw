#pragma once
#ifndef _GAMESTATE_HPP_
#define _GAMESTATE_HPP_
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <string>
#include <map>
#include <vector>

class GameObject;
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
	GameObject* target;
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
	: id(-1), target(nullptr), display(Show)
	{ }
};

struct GameState
{
	unsigned int currentProgress;
	unsigned int maxProgress;
	unsigned int numMissions;
	unsigned int numHiddenPackages;
	unsigned int numHiddenPackagesDiscovered;
	unsigned int numUniqueJumps;
	unsigned int numRampages;
	unsigned int maxWantedLevel;
	PlayerController* player;

	unsigned int currentWeather;

	/**
	 * @brief Stores a pointer to script global that stores the on-mission state.
	 */
	unsigned int *scriptOnMissionFlag;
	
	/** Objects created by mission scripts */
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

	short hour;
	short minute;
	
	std::string lastMissionName;
	
	/// Stores the "special" character and cutscene model indices.
	std::map<unsigned short, std::string> specialCharacters;
	std::map<unsigned short, std::string> specialModels;

	std::vector<OnscreenText> text;
	
	TextDisplayData nextText;
	std::vector<TextDisplayData> texts;

	/** The camera near value currently set by the script */
	float cameraNear;
	bool cameraFixed;
	glm::vec3 cameraPosition;
	glm::quat cameraRotation;
	
	GameObject* cameraTarget;
	
	std::vector<VehicleGenerator> vehicleGenerators;
	
	std::map<int, BlipData> radarBlips;

	GameState() :
		currentProgress(0),
		maxProgress(1),
		numMissions(0),
		numHiddenPackages(0),
		numHiddenPackagesDiscovered(0),
		numUniqueJumps(0),
		numRampages(0),
		maxWantedLevel(0),
		player(nullptr),
		currentWeather(0),
		scriptOnMissionFlag(nullptr),
		fadeOut(true),
		fadeStart(0.f),
		fadeTime(0.f),
		fadeSound(false),
		skipCutscene(false),
		isIntroPlaying(false),
		currentCutscene(nullptr),
		cutsceneStartTime(-1.f),
		isCinematic(false),
		hour(0),
		minute(0),
		cameraNear(0.1f),
		cameraFixed(false),
		cameraTarget(nullptr)
	{}
	
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
