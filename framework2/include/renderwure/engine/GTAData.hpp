#pragma once
#ifndef _GTADATA_HPP_
#define _GTADATA_HPP_

#include <renderwure/engine/GTATypes.hpp>
#include <renderwure/loaders/LoaderIMG.hpp>
#include <renderwure/loaders/TextureLoader.hpp>
#include <renderwure/loaders/LoaderDFF.hpp>
#include <renderwure/loaders/LoaderIDE.hpp>
#include <renderwure/loaders/LoaderIFP.hpp>
#include <renderwure/loaders/WeatherLoader.hpp>
#include <renderwure/loaders/LoaderCOL.hpp>

#include <memory>

class GTAEngine;
class TextureAtlas;

/**
 * @brief The TextureInfo struct
 * Contains metadata about where a texture can be found.
 */
struct TextureInfo
{
	/// Texture Name
	GLuint texName;
	/// Atlas (if applicable)
	TextureAtlas* atlas;
	glm::vec4 rect; /// X/Y base coord, Z/W UV scale.

	TextureInfo(GLuint tex, TextureAtlas* a, const glm::vec4&r)
		: texName(tex), atlas(a), rect(r) {}
	TextureInfo()
		: texName(0), atlas(nullptr) {}
};

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
	
	GTAEngine* engine;
	
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
     * Loads an IFP file containing animations
     */
    void loadIFP(const std::string& name);
	
	/**
	 * Returns a pointer to the named file if it is available, the memory must be freed.
	 * @param name the filename in the archive
	 * @return pointer to the data, NULL if it is not available 
	 */
	char* loadFile(const std::string& name);

	/**
	 * @brief getAtlas Returns atlas i, creating it if the situation calls for it.
	 *  "the situation" being the last atlas has more than 0 textures packed.
	 * @param i
	 * @return
	 */
	TextureAtlas* getAtlas(size_t i);
	
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
    std::map<std::string, Model*> models;

	/**
	 * Loaded Textures and their atlas entries.
	 */
	std::map<std::string, TextureInfo> textures;

	/**
	 * Texture atlases.
	 */
	std::vector<TextureAtlas*> atlases;

    /**
     * Loaded Animations
     */
    AnimationSet animations;

	/**
	 * Loaded collision proxies
	 */
	std::map<std::string,  std::unique_ptr<CollisionInstance>> collisions;
	
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
