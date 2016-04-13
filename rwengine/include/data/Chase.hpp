#ifndef _CHASE_HPP_
#define _CHASE_HPP_

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <vector>
#include <map>

class GameObject;

struct ChaseKeyframe
{
	glm::vec3 velocity;
	int steeringAngle;
	int acceleratorPower;
	int brakePower;
	bool handbrake;
	glm::vec3 position;
	glm::quat rotation;

	static bool load(const std::string& filePath, std::vector<ChaseKeyframe>& frames);
};

/**
 * @brief The ChaseCoordinator class handles loading and playing a chase
 *
 * It reads in ChaseKeyframes for a set of objects, and replays them
 * over time.
 */
class ChaseCoordinator
{
public:

	ChaseCoordinator()
		: chaseTime(-1.f)
	{ }

	bool addChaseVehicle(GameObject* vehicle, int index, const std::string& pathFile);
	GameObject* getChaseVehicle(int index);
	void removeChaseVehicle(int index);

	void start();
	void update(float dt);

	void cleanup();
private:
	float chaseTime;
	struct ChaseObject {
		std::vector<ChaseKeyframe> keyframes;
		GameObject* object;
	};

	std::map<int, ChaseObject> chaseVehicles;
};

#endif
