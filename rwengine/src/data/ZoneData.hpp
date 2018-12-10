#ifndef _RWENGINE_ZONEDATA_HPP_
#define _RWENGINE_ZONEDATA_HPP_

#include <glm/vec3.hpp>

#include <memory>
#include <string>
#include <utility>
#include <vector>

#define ZONE_GANG_COUNT 13

/**
 * Zone Data loaded from IPL/zon files
 */
struct ZoneData {
    /**
     * The name of the Zone (see .gxt)
     */
    std::string name{};

    int type{};

    /**
     * Bottom left of the Zone
     */
    glm::vec3 min{};

    /**
     * Top Right of the zone
     */
    glm::vec3 max{};

    /**
     * Island number
     */
    int island{};

    /**
     * Text of the zone?
     */
    std::string text = {};

    /**
     * Gang spawn density for daytime (8:00-19:00)
     */
    unsigned int gangDensityDay[ZONE_GANG_COUNT] = {};

    /**
     * Gang spawn density for nighttime (19:00-8:00)
     */
    unsigned int gangDensityNight[ZONE_GANG_COUNT] = {};

    /**
     * Gang car spawn density for daytime (8:00-19:00)
     */
    unsigned int gangCarDensityDay[ZONE_GANG_COUNT] = {};

    /**
     * Gang car spawn density for nighttime (19:00-8:00)
     */
    unsigned int gangCarDensityNight[ZONE_GANG_COUNT] = {};

    unsigned int pedGroupDay = 0;
    unsigned int pedGroupNight = 0;

    /**
     * Pointer to parent zone in zone array
     */
    ZoneData* parent_ = nullptr;

    /**
     * Totally contained zones
     */
    std::vector<ZoneData*> children_ = {};

    template <class String>
    ZoneData(String&& _name, const int& _type, const glm::vec3& _min,
             const glm::vec3& _max, const int& _island,
             const unsigned int& _pedGroupDay,
             const unsigned int& _pedGroupNight)
        : name(std::forward<String>(_name))
        , type(_type)
        , min(_min)
        , max(_max)
        , island(_island)
        , pedGroupDay(_pedGroupDay)
        , pedGroupNight(_pedGroupNight) {
    }

    ZoneData() = default;

    static bool isZoneContained(const ZoneData& inner, const ZoneData& outer);

    bool containsPoint(const glm::vec3& point) const;

    ZoneData* findLeafAtPoint(const glm::vec3& point);

    bool insertZone(ZoneData& inner);
};

using ZoneDataList = std::vector<ZoneData>;

#endif
