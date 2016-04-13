#include "data/Chase.hpp"
#include <fstream>
#include <cstdint>
#include <rw/defines.hpp>
#include <objects/GameObject.hpp>
#include <engine/GameWorld.hpp>

#define KEYFRAMES_PER_SECOND 30

bool ChaseKeyframe::load(const std::string &filePath, std::vector<ChaseKeyframe> &frames)
{
	std::ifstream chaseFile(filePath, std::ios_base::binary);
	RW_CHECK(chaseFile.is_open(), "Failed to open chase file");
	if (!chaseFile.is_open()) {
		return false;
	}

	chaseFile.seekg(0, std::ios_base::end);
	size_t fileLength = chaseFile.tellg();
	chaseFile.seekg(0);

	struct ChaseEntryRecord {
		int16_t velocity[3];
		int8_t right[3];
		int8_t up[3];
		uint8_t steering;
		uint8_t driving;
		uint8_t braking;
		uint8_t handbrake;
		glm::vec3 position;
	};

	static_assert(sizeof(ChaseEntryRecord) == 28, "ChaseEntryRecord is not 28 bytes");
	RW_CHECK(fileLength % sizeof(ChaseEntryRecord) == 0, "File is not a mulitple of 28 byte");

	size_t recordCount = fileLength / sizeof(ChaseEntryRecord);

	for (size_t i = 0; i < recordCount; ++i)
	{
		ChaseEntryRecord rec;
		chaseFile.read((char*)&rec, sizeof(ChaseEntryRecord));
		glm::vec3 velocity {
							 rec.velocity[0]/16383.5f,
							 rec.velocity[1]/16383.5f,
							 rec.velocity[2]/16383.5f,
						 };
		glm::vec3 right{
							 rec.right[0]/127.5f,
							 rec.right[1]/127.5f,
							 rec.right[2]/127.5f,
						 };
		glm::vec3 up{
							 rec.up[0]/127.5f,
							 rec.up[1]/127.5f,
							 rec.up[2]/127.5f,
						 };
		glm::mat3 rotation(right, up, glm::cross(right, up));
		frames.push_back({
							 velocity,
							 rec.steering,
							 rec.driving,
							 rec.braking,
							 !!rec.handbrake,
							 rec.position,
							 glm::quat_cast(rotation)
						 });
	}

	return true;
}




bool ChaseCoordinator::addChaseVehicle(GameObject *vehicle, int index, const std::string &pathFile)
{
	std::vector <ChaseKeyframe> keyframes;
	bool result = ChaseKeyframe::load(pathFile, keyframes);
	RW_CHECK(result, "Failed to load chase keyframes: " + pathFile);
	chaseVehicles[index] = {keyframes, vehicle};
	return result;
}

GameObject*ChaseCoordinator::getChaseVehicle(int index)
{
	return chaseVehicles.at(index).object;
}

void ChaseCoordinator::removeChaseVehicle(int index)
{
	auto it = chaseVehicles.find(index);
	RW_CHECK(it != chaseVehicles.end(), "Vehicle not in chase");
	if (it != chaseVehicles.end())
	{
		chaseVehicles.erase(it);
	}
}

void ChaseCoordinator::start()
{
	chaseTime = 0.f;
}

void ChaseCoordinator::update(float dt)
{
	chaseTime += dt;
	size_t frameNum = chaseTime * KEYFRAMES_PER_SECOND;
	for(auto& it : chaseVehicles)
	{
		RW_CHECK(frameNum < it.second.keyframes.size(), "Vehicle out of chase keyframes");
		if (frameNum >= it.second.keyframes.size()) continue;

		const ChaseKeyframe& kf = it.second.keyframes[frameNum];
		it.second.object->setPosition(kf.position);
		it.second.object->setRotation(kf.rotation);
	}
}

void ChaseCoordinator::cleanup()
{
	for(auto& it : chaseVehicles)
	{
		it.second.object->engine->destroyObject(it.second.object);
	}
	chaseVehicles.clear();
}

