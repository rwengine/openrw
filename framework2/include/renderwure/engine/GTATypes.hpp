#pragma once
#ifndef _GTATYPES_HPP_
#define _GTATYPES_HPP_

#include <cstdint>
#include <map>
#include <string>

class Animation;

typedef std::map<std::string, Animation*> AnimationSet;

namespace GTATypes
{

/**
 * @struct RGB
 *  Stores 8 bit RGB data
 */
struct RGB
{
	uint8_t r, g, b;
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
