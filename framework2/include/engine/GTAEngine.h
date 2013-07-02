#ifndef _GTAENGINE_H_
#define _GTAENGINE_H_
#include "GTAData.h"
#include <loaders/LoaderIPL.h>
#include <vector>
#include <glm/glm.hpp>

/**
 * @class GTAEngine
 *  Provides a simple interface to the framework's internals
 */
class GTAEngine
{
public:
	
	GTAEngine(const std::string& gamepath);
	
	/**
	 * Loads the game data
	 */
	bool load();
	
	/**
	 * Loads an IPL into the game.
	 * @param name The name of the IPL as it appears in the games' gta.dat
	 */
	bool loadItems(const std::string& name);
	
	/**
	 * Roughly the middle of everything
	 */
	glm::vec3 itemCentroid;
	
	/**
	 * Game data
	 */
	GTAData gameData;
	
	/**
	 * Until we have a real "object" class, just store a list of loaed instances.
	 */
	std::vector<LoaderIPLInstance> instances;
};

#endif