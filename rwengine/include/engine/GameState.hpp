#pragma once
#ifndef _GAMESTATE_HPP_
#define _GAMESTATE_HPP_
#include <glm/glm.hpp>

class PlayerController;

struct GameState
{
	unsigned int maxProgress;
	unsigned int numMissions;
	unsigned int numHiddenPackages;
	unsigned int numUniqueJumps;
	unsigned int numRampages;
	unsigned int maxWantedLevel;
	PlayerController* player;

	glm::i8vec3 fadeColour;

	/**
	 * @brief Stores a pointer to script global that stores the on-mission state.
	 */
	unsigned int *scriptOnMissionFlag;

	bool fadeEnabled;
	bool fadeIn;
	float fadeTime;

	bool isIntroPlaying;
};

#endif
