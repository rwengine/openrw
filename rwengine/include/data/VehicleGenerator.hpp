#ifndef RWENGINE_VEHICLEGENERATOR_HPP
#define RWENGINE_VEHICLEGENERATOR_HPP

/**
 * Stores information about where the game can generate vehicles.
 */
struct VehicleGenerator
{
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
};

#endif
