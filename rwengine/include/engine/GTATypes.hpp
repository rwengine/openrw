#pragma once
#ifndef _GTATYPES_HPP_
#define _GTATYPES_HPP_

#include <cstdint>
#include <map>
#include <string>
#include <glm/glm.hpp>

class Animation;

typedef std::map<std::string, Animation*> AnimationSet;

class Model;

// TODO: Make generic.
struct ModelHandle {
	Model* model;
	std::string name;

	ModelHandle(const std::string& name) : model( nullptr ), name(name) {}
};

namespace GTATypes
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

/**
 * @struct WaterRect
 *  Rectangle with water information
 */
struct WaterRect
{
	float height;
	float xLeft, yBottom;
	float xRight, yTop;
};
}

#endif
