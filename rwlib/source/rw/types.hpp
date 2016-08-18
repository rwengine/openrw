#pragma once
#ifndef _RWTYPES_HPP_
#define _RWTYPES_HPP_

#include <cstdint>
#include <map>
#include <string>
#include <glm/glm.hpp>
#include <memory>
#include <rw/defines.hpp>

#define RW_USING(feature) 1 == feature

#define NO_WATER_INDEX 48
#define WATER_LQ_DATA_SIZE 64
#define WATER_HQ_DATA_SIZE 128
#define WATER_WORLD_SIZE 4096.f
#define WATER_HQ_DISTANCE 128.f
#define WATER_SCALE 0.05f
#define WATER_HEIGHT 0.5f
#define WATER_BUOYANCY_K 1500.f
#define WATER_BUOYANCY_C 1000.f

/**
 * These control the size of grid that is applied to
 * AI Graph and culling.
 */
#define WORLD_GRID_SIZE (4000l)
#define WORLD_CELL_SIZE (100l)
#define WORLD_GRID_WIDTH (WORLD_GRID_SIZE / WORLD_CELL_SIZE)
#define WORLD_GRID_CELLS (WORLD_GRID_WIDTH * WORLD_GRID_WIDTH)

struct Animation;

typedef std::map<std::string, Animation*> AnimationSet;

namespace RWTypes
{
/**
 * @struct RGB
 *  Stores 8 bit RGB data
 */
struct RGB {
  uint8_t r, g, b;

  operator glm::vec3() { return glm::vec3(r, g, b) / 255.f; }
};

/**
 * @struct RGBA
 *  Stores 8 bit RGBA data
 */
struct RGBA {
  uint8_t r, g, b, a;
};
}

#endif
