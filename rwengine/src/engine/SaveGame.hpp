#pragma once
#ifndef _SAVEGAME_HPP_
#define _SAVEGAME_HPP_

#include <rw/types.hpp>

#include <engine/GameState.hpp>

#include <string>
#include <vector>

class GameWorld;
class ScriptMachine;

struct SaveGameInfo {
  std::string savePath;
  bool valid;
  BasicState basicState;
};

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

  static bool getSaveInfo(const std::string& file, BasicState* outState);

  /**
   * Returns save game information for all found saves
   */
  static std::vector<SaveGameInfo> getAllSaveGameInfo();
};

#endif
