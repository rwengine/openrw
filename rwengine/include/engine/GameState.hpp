#pragma once
#ifndef _GAMESTATE_HPP_
#define _GAMESTATE_HPP_
#include <glm/glm.hpp>
#include <string>
#include <map>

class PlayerController;
class CutsceneData;

struct GameState
{
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

	bool fadeOut;
	float fadeStart;
	float fadeTime;
	bool fadeSound;
	glm::u16vec3 fadeColour;


	bool isIntroPlaying;
	CutsceneData* currentCutscene;
	float cutsceneStartTime;

	short hour;
	short minute;

	/// @todo opcodes seem to imply Onscreen text might be queued
	unsigned short osTextStyle;
	std::string osTextString;
	float osTextStart;
	float osTextTime;

	/// Stores the "special" character and cutscene model indices.
	std::map<unsigned short, std::string> specialCharacters;
	std::map<unsigned short, std::string> specialModels;

	GameState() :
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
		fadeOut(false),
		fadeStart(0.f),
		fadeTime(0.f),
		fadeSound(false),
		isIntroPlaying(false),
		currentCutscene(nullptr),
		cutsceneStartTime(-1.f),
		hour(0),
		minute(0),
		osTextStyle(0),
		osTextStart(0.f),
		osTextTime(0.f)
	{}
};

#endif
