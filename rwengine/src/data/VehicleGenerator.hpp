#ifndef RWENGINE_VEHICLEGENERATOR_HPP
#define RWENGINE_VEHICLEGENERATOR_HPP
#include <glm/glm.hpp>

/**
 * Stores information about where the game can generate vehicles.
 */
struct VehicleGenerator
{
	/// Script reference ID
	size_t generatorID;
	glm::vec3 position;
	float heading;
	/// ID of the vehicle to spawn, or -1 for random.
	int vehicleID;
	/// @todo not yet used
	int colourFG;
	/// @todo not yet used
	int colourBG;
	bool alwaysSpawn;
	/// @todo not yet used
	short alarmThreshold;
	/// @todo not yet used
	short lockedThreshold;

	int minDelay;
	/// @todo not yet used
	int maxDelay;
	int lastSpawnTime;

	/**
	 * Number of vehicles left to spawn 0-100, 101 = never decrement.
	 * Intentionally disabled to match behaviour
	*/
	int remainingSpawns;

	VehicleGenerator(size_t id,
					 const glm::vec3& position_,
					 float heading_,
					 int modelID_,
					 int colourFG_,
					 int colourBG_,
					 bool alwaysSpawn_,
					 short alarmThreshold_,
					 short lockedThreshold_,
					 int minDelay_,
					 int maxDelay_,
					 int lastSpawnTime_,
					 int remainingSpawns_)
		: generatorID(id)
		, position(position_)
		, heading(heading_)
		, vehicleID(modelID_)
		, colourFG(colourFG_)
		, colourBG(colourBG_)
		, alwaysSpawn(alwaysSpawn_)
		, alarmThreshold(alarmThreshold_)
		, lockedThreshold(lockedThreshold_)
		, minDelay(minDelay_)
		, maxDelay(maxDelay_)
		, lastSpawnTime(lastSpawnTime_)
		, remainingSpawns(remainingSpawns_)
	{}

	int getScriptObjectID() const
	{
		return generatorID;
	}
};

#endif
