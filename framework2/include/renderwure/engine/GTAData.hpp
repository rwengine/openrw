#pragma once
#ifndef _GTADATA_HPP_
#define _GTADATA_HPP_

#include <renderwure/loaders/LoaderIMG.hpp>
#include <renderwure/loaders/TextureLoader.hpp>
#include <renderwure/loaders/LoaderDFF.hpp>
#include <renderwure/loaders/LoaderIDE.hpp>
#include <renderwure/loaders/WeatherLoader.hpp>

#include <string>
#include <map>
#include <memory>

/**
 * Handles loading and management of the Game's DAT
 */
class GTAData
{
private:
	
	std::string datpath;
	std::string splash;
	
public:
	
	/**
	 * @struct GTAFile
	 *  Stores information about a file the engine might want to load
	 */
	struct GTAFile
	{
		bool archived; /// Is the file inside an IMG or on the filesystem?
		std::string path; /// Path to the file containing the file
	};
	
	/**
	 * ctor
	 * @param path Path to the root of the game data.
	 */
	GTAData(const std::string& path);
	
	/**
	 * Returns the current platform
	 */
	std::string getPlatformString()
	{
		return "PC";
	}
	
	/**
	 * Loads the data contained in the given file
	 */
	void loadIDE(const std::string& name);
	
	/**
	 * Handles the parsing of a COL file.
	 */
	void loadCOL(const size_t zone, const std::string& name);
	
	/**
	 * Handles the loading of an IMG's data
	 */
	void loadIMG(const std::string& name);
	
	void loadIPL(const std::string& name);
	
	void loadCarcols(const std::string& path);

	void loadWeather(const std::string& path);
	
	/**
	 * Loads water level data
	 */
	void loadWaterpro(const std::string& path);
	void loadWater(const std::string& path);
	
	void load();
	
	/**
	 * Loads a GTA3.dat file with the name path
	 */
	void parseDAT(const std::string& path);
	
	/**
	 * Attempts to load a TXD, or does nothing if it has already been loaded
	 */
	void loadTXD(const std::string& name);
	
	/**
	 * Attempts to load a DFF or does nothing if is already loaded
	 */
	void loadDFF(const std::string& name);
	
	/**
	 * Returns a pointer to the named file if it is available, the memory must be freed.
	 * @param name the filename in the archive
	 * @return pointer to the data, NULL if it is not available 
	 */
	char* loadFile(const std::string& name);
	
	/**
	 * Files that have been loaded previously
	 */
	std::map<std::string, bool> loadedFiles;
	
	/**
	 * Maps the paths in GTA3.dat to the real paths
	 */
	std::map<std::string, std::string> iplLocations;
	std::map<std::string, std::string> ideLocations;
	
	/**
	 * Maps file names to their locations
	 */
	std::map<std::string, GTAFile> fileLocations;
	
	/**
	 * Map of loaded archives
	 */
	std::map<std::string, LoaderIMG> archives;
	
	/**
	 * The vehicle colour palettes
	 */
	std::vector<glm::vec3> vehicleColours;
	
	/**
	 * The vehicle colours for each vehicle type
	 */
	std::map<std::string, std::vector<std::pair<size_t,size_t>>> vehiclePalettes;
	
	/**
	 * Texture Loader
	 */
	TextureLoader textureLoader;

	/**
	 * Weather Loader
	 */
	WeatherLoader weatherLoader;
	
	/**
	 * Loaded models
	 */
	std::map<std::string, std::unique_ptr<Model>> models;
	
	/**
	 * Water Rectangles
	 */
	std::vector<GTATypes::WaterRect> waterRects;
	
	/**
	 * Water heights
	 */
	float waterHeights[48];
	
	/**
	 * Visible water heights
	 */
	uint8_t visibleWater[64*64];
	
	/**
	 * The "real" water heights
	 */
	uint8_t realWater[128*128];
};

#endif