#pragma once
#ifndef _GAMEDATA_HPP_
#define _GAMEDATA_HPP_

class Logger;

#include <rw/types.hpp>
#include <loaders/LoaderIMG.hpp>
#include <loaders/LoaderTXD.hpp>
#include <loaders/LoaderDFF.hpp>
#include <loaders/LoaderIDE.hpp>
#include <loaders/LoaderIFP.hpp>
#include <loaders/WeatherLoader.hpp>
#include <objects/VehicleInfo.hpp>
#include <data/CollisionModel.hpp>
#include <data/GameTexts.hpp>
#include <data/ZoneData.hpp>

#include <audio/MADStream.hpp>
#include <gl/TextureData.hpp>
#include <platform/FileIndex.hpp>

#include <memory>

struct DynamicObjectData;
struct WeaponData;
class GameWorld;
class TextureAtlas;
class SCMFile;

/**
 * @brief Loads and stores all "static" data such as loaded models, handling
 * information, weather, object definitions etc.
 *
 * @todo Move parsing of one-off data files from this class.
 * @todo Improve how Loaders and written and used
 * @todo Considering implementation of streaming data and object handles.
 */
class GameData
{
private:
	
	std::string datpath;
	std::string splash;
	
	Logger* logger;
	WorkContext* workContext;
public:

	/**
	 * ctor
	 * @param path Path to the root of the game data.
	 */
	GameData(Logger* log, WorkContext* work, const std::string& path = "");
	~GameData();
	
	GameWorld* engine;
	
	/**
	 * Returns the current platform
	 */
	std::string getPlatformString()
	{
		return "PC";
	}
	
	/**
	 * Returns the game data path
	 */
	const std::string& getDataPath() const
	{
		return datpath;
	}
	
	/**
	 * Loads items defined in the given IDE
	 */
	void loadIDE(const std::string& path);

	/**
	 * Handles the parsing of a COL file.
	 */
	void loadCOL(const size_t zone, const std::string& name);
	
	/**
	 * Handles the loading of an IMG's data
	 */
	void loadIMG(const std::string& name);
	
	void loadIPL(const std::string& path);
	
	/**
	 * Loads the Zones from a zon/IPL file
	 */
	bool loadZone(const std::string& path);
		
	void loadCarcols(const std::string& path);

	void loadWeather(const std::string& path);

	void loadHandling(const std::string& path);

	SCMFile* loadSCM(const std::string& path);

	void loadGXT(const std::string& name);
	
	/**
	 * Loads water level data
	 */
	void loadWaterpro(const std::string& path);
	void loadWater(const std::string& path);
	
	void load();
	
	/**
	 * Loads model, placement, models and textures from a level file
	 */
	void loadLevelFile(const std::string& path);
	
	/**
	 * Attempts to load a TXD, or does nothing if it has already been loaded
	 */
	void loadTXD(const std::string& name, bool async = false);

	/**
	 * Attempts to load a DFF or does nothing if is already loaded
	 */
	void loadDFF(const std::string& name, bool async = false);

    /**
     * Loads an IFP file containing animations
     */
    void loadIFP(const std::string& name);
	
	/**
	 * Loads data from an object definition dat.
	 */
	void loadDynamicObjects(const std::string& name);

	/**
	 * Loads weapon.dat
	 */
	void loadWeaponDAT(const std::string& path);

	bool loadAudioStream(const std::string& name);
	bool loadAudioClip(const std::string& name, const std::string& fileName);

	void loadSplash(const std::string& name);

	TextureData::Handle findTexture( const std::string& name, const std::string& alpha = "" )
	{
		return textures[{name, alpha}];
	}
	
	FileIndex index;
	
	/**
	 * Files that have been loaded previously
	 */
	std::map<std::string, bool> loadedFiles;
	
	/**
	 * IPL file locations
	 */
	std::map<std::string, std::string> iplLocations;

	/**
	 * Map of loaded archives
	 */
	std::map<std::string, LoaderIMG> archives;
	
	/**
	 * Map Zones
	 */
	std::map<std::string, ZoneData> zones;

	/**
	 * Object Definitions
	 */
	std::map<ObjectID, ObjectInformationPtr> objectTypes;

	uint16_t findModelObject(const std::string model);

	template<class T> std::shared_ptr<T> findObjectType(ObjectID id)
	{
		auto f = objectTypes.find(id);
		/// @TODO don't instanciate an object here just to read .type
		T tmp;
		if( f != objectTypes.end() && f->second->class_type == tmp.class_type )
		{
			return std::static_pointer_cast<T>(f->second);
		}
		return nullptr;
	}

	/**
	 * The vehicle colour palettes
	 */
	std::vector<glm::u8vec3> vehicleColours;
	
	/**
	 * The vehicle colours for each vehicle type
	 */
	std::map<std::string, std::vector<std::pair<size_t,size_t>>> vehiclePalettes;

	/**
	 * Vehicle information
	 */
	std::map<std::string, VehicleInfoHandle> vehicleInfo;
	
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
	std::map<std::string, ResourceHandle<Model>::Ref> models;

	/**
	 * Loaded textures (Textures are ID by name and alpha pairs)
	 */
	std::map<std::pair<std::string, std::string>, TextureData::Handle> textures;

	/**
	 * Texture atlases.
	 */
	std::vector<TextureAtlas*> atlases;

    /**
     * Loaded Animations
     */
    AnimationSet animations;

	/**
	 * CollisionModel data.
	 */
	std::map<std::string,  std::unique_ptr<CollisionModel>> collisions;
	
	/**
	 * DynamicObjectData 
	 */
	std::map<std::string, std::shared_ptr<DynamicObjectData>> dynamicObjectData;
	
	std::vector<std::shared_ptr<WeaponData>> weaponData;

	/**
	 * @struct WaterArea
	 *  Stores Water Rectangle Information
	 */
	struct WaterArea
	{
		float height;
		float xLeft, yBottom;
		float xRight, yTop;
	};

	/**
	 * Water Areas
	 */
	std::vector<WaterArea> waterBlocks;
	
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

	int getWaterIndexAt(const glm::vec3& ws) const;
	float getWaveHeightAt(const glm::vec3& ws) const;

	GameTexts texts;
	
	/**
	 * Determines whether the given path is a valid game directory.
	 */
	static bool isValidGameDirectory(const std::string& path);
};

#endif
