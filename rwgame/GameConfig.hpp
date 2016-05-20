#ifndef RWGAME_GAMECONFIG_HPP
#define RWGAME_GAMECONFIG_HPP
#include <string>

class GameConfig
{
public:
	/**
	 * @brief GameConfig Loads a game configuration
	 * @param configName The configuration filename to load
	 * @param configPath Where to look.
	 */
	GameConfig(const std::string& configName, const std::string& configPath = getDefaultConfigPath());

	/**
	 * @brief getFilePath Returns the system file path for the configuration
	 */
	std::string getConfigFile();

	/**
	 * @brief isValid
	 * @return True if the loaded configuration is valid
	 */
	bool isValid();

	const std::string& getGameDataPath() const { return m_gamePath; }

private:
	static std::string getDefaultConfigPath();
	static int handler(void*, const char*, const char*, const char*);

	/* Config State */
	std::string m_configName;
	std::string m_configPath;
	bool m_valid;

	/* Actual Configuration */

	/// Path to the game data
	std::string m_gamePath;
};

#endif
