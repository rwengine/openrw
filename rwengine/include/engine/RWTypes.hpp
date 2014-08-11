#pragma once
#ifndef _RWTYPES_HPP_
#define _RWTYPES_HPP_

#include <cstdint>
#include <map>
#include <string>
#include <glm/glm.hpp>
#include <memory>

#define NO_WATER_INDEX 48
#define WATER_LQ_DATA_SIZE 64
#define WATER_HQ_DATA_SIZE 128
#define WATER_WORLD_SIZE 4096.f
#define WATER_HQ_DISTANCE 128.f
#define WATER_SCALE 0.05f
#define WATER_HEIGHT 0.5f
#define WATER_BUOYANCY_K 25000.f
#define WATER_BUOYANCY_C 2550.f

struct Animation;

typedef std::map<std::string, Animation*> AnimationSet;

class Model;

// TODO: Make generic.
struct ModelHandle {
	Model* model;
	std::string name;

	ModelHandle(const std::string& name) : model( nullptr ), name(name) {}
};

namespace RWTypes
{

/**
 * @struct RGB
 *  Stores 8 bit RGB data
 */
struct RGB
{
	uint8_t r, g, b;

    operator glm::vec3() {
        return glm::vec3(r, g, b)/255.f;
    }
};

/**
 * @struct RGBA
 *  Stores 8 bit RGBA data
 */
struct RGBA
{
	uint8_t r, g, b, a;
};
}

/**
 * @brief Returned by GameData::loadFile()
 */
struct FileContentsInfo {
	char* data;
	size_t length;

	~FileContentsInfo() {
		delete[] data;
	}
};
typedef std::shared_ptr<FileContentsInfo> FileHandle;

#endif
