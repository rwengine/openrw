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

	static void writeState(GameState& state, const std::string& file);

	static bool loadState(GameState& state, const std::string& file);

	static void writeScript(ScriptMachine& sm, const std::string& file);

	static bool loadScript(ScriptMachine&, const std::string& file);

	static void writeObjects(GameWorld& world, const std::string& file);

	static bool loadObjects(GameWorld& world, const std::string& file);
};

#endif