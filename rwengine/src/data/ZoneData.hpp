#ifndef _RWENGINE_ZONEDATA_HPP_
#define _RWENGINE_ZONEDATA_HPP_

#include <algorithm>
#include <glm/glm.hpp>
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

    static bool isZoneContained(const ZoneData& inner, const ZoneData& outer) {
        return glm::all(glm::greaterThanEqual(inner.min, outer.min)) &&
               glm::all(glm::lessThanEqual(inner.max, outer.max));
    }

    bool containsPoint(const glm::vec3& point) const {
        return glm::all(glm::greaterThanEqual(point, min)) &&
               glm::all(glm::lessThanEqual(point, max));
    }

    ZoneData* findLeafAtPoint(const glm::vec3& point) {
        for (ZoneData* child : children_) {
            auto descendent = child->findLeafAtPoint(point);
            if (descendent) {
                return descendent;
            }
        }
        return containsPoint(point) ? this : nullptr;
    }

    bool insertZone(ZoneData& inner) {
        if (!isZoneContained(inner, *this)) {
            return false;
        }

        for (ZoneData* child : children_) {
            if (child->insertZone(inner)) {
                return true;
            }
        }

        // inner is a child of outer

        // Move any zones that are really within inner to inner
        auto it = std::stable_partition(
            children_.begin(), children_.end(),
            [&](ZoneData* a) { return !inner.insertZone(*a); });
        children_.erase(it, children_.end());

        children_.push_back(&inner);
        inner.parent_ = this;

        return true;
    }
};

using ZoneDataList = std::vector<ZoneData>;

#endif
