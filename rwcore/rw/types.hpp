#ifndef _LIBRW_TYPES_HPP_
#define _LIBRW_TYPES_HPP_

#include <cstdint>
#include <map>
#include <memory>
#include <rw/debug.hpp>
#include <string>

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

#endif
