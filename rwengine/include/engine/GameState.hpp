#pragma once
#ifndef _GAMESTATE_HPP_
#define _GAMESTATE_HPP_
#include <glm/glm.hpp>
#include <string>

class PlayerController;

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

	glm::i8vec3 fadeColour;

	unsigned int currentWeather;

	/**
	 * @brief Stores a pointer to script global that stores the on-mission state.
	 */
	unsigned int *scriptOnMissionFlag;

	bool fadeIn;
	float fadeStart;
	float fadeTime;
	bool fadeSound;

	bool isIntroPlaying;
	float cutsceneStartTime;

	short hour;
	short minute;

	/// @todo opcodes seem to imply Onscreen text might be queued
	unsigned short osTextStyle;
	std::string osTextString;
	float osTextStart;
	float osTextTime;

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
		fadeIn(false),
		fadeStart(0.f),
		fadeTime(0.f),
		fadeSound(false),
		isIntroPlaying(false),
		cutsceneStartTime(0.f),
		hour(0),
		minute(0),
		osTextStyle(0),
		osTextStart(0.f),
		osTextTime(0.f)
	{}
};

#endif
