#pragma once
#ifndef _SAVEGAME_HPP_
#define _SAVEGAME_HPP_

#include <engine/RWTypes.hpp>

#include <string>

struct GameState;
class GameWorld;
class ScriptMachine;

/**
 * Reads and Writes GameStates to disk, restoring the required state information
 */
class SaveGame
{
public:

	/**
	 * Writes the entire game state to a file format that closely approximates
	 * the format used in GTA III
	 */
	static void writeGame(GameState& state, const std::string& file);

	/**
	 * Loads an entire Game State from a file, using a format similar to the
	 * format used by GTA III.
	 * 
	 * It assumes that the state already has a world and script that have been
	 * initalized to the same state as the game being loaded.
	 * @return status, false if failure occured.
	 */
	static bool loadGame(GameState& state, const std::string& file);

	/**
	 * Writes the current game state out into a file suitable for loading later.
	 */
	static void writeState(GameState& state, const std::string& file);

	static bool loadState(GameState& state, const std::string& file);

	static void writeScript(ScriptMachine& sm, const std::string& file);

	static bool loadScript(ScriptMachine&, const std::string& file);

	static void writeObjects(GameWorld& world, const std::string& file);

	static bool loadObjects(GameWorld& world, const std::string& file);
};

#endif