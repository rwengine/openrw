#pragma once
#ifndef _ZONEDATA_HPP_
#define _ZONEDATA_HPP_

#include <glm/glm.hpp>
#include <string>

#define ZONE_GANG_COUNT 13

/**
 * \class Zone
 *  A Zone entry
 */
struct ZoneData {
  /**
  * The name of the Zone (see .gxt)
  */
  std::string name;

  int type;

  /**
  * Bottom left of the Zone
  */
  glm::vec3 min;

  /**
  * Top Right of the zone
  */
  glm::vec3 max;

  /**
  * Island number
  */
  int island;

  /**
  * Text of the zone?
  */
  std::string Text;

  /**
   * Gang spawn density for daytime (8:00-19:00)
   */
  unsigned int gangDensityDay[ZONE_GANG_COUNT];

  /**
   * Gang spawn density for nighttime (19:00-8:00)
   */
  unsigned int gangDensityNight[ZONE_GANG_COUNT];

  /**
   * Gang car spawn density for daytime (8:00-19:00)
   */
  unsigned int gangCarDensityDay[ZONE_GANG_COUNT];

  /**
   * Gang car spawn density for nighttime (19:00-8:00)
   */
  unsigned int gangCarDensityNight[ZONE_GANG_COUNT];

  unsigned int pedGroupDay;
  unsigned int pedGroupNight;
};

#endif