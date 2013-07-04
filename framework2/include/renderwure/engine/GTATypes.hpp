#pragma once
#ifndef _GTATYPES_HPP_
#define _GTATYPES_HPP_

#include <cstdint>

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

}

#endif