#pragma once
#ifndef _GTAENGINE_HPP_
#define _GTAENGINE_HPP_

#include <renderwure/engine/GTAData.hpp>
#include <renderwure/loaders/LoaderIPL.hpp>
#include <renderwure/render/GTARenderer.hpp>

#include <glm/glm.hpp>

#include <vector>
#include <queue>

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
	
	struct LogEntry
	{
		enum Type {
			Info,
			Error,
			Warning
		};
		
		Type type;
		float time;
		std::string message;
	};
	
	std::queue<LogEntry> log;
	
	/**
	 * Displays an informative message
	 */
	void logInfo(const std::string& info);
	
	/**
	 * Displays an alarming error
	 */
	void logError(const std::string& error);
	
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
	 * Game Clock
	 */
	float gameTime;
	
	/**
	 * Game data
	 */
	GTAData gameData;
	
	/**
	 * Renderer
	 */
	GTARenderer renderer;
	
	/**
	 * Until we have a real "object" class, just store a list of loaed instances.
	 */
	std::vector<LoaderIPLInstance> instances;
};

#endif