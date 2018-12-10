#include "data/ZoneData.hpp"

#include "glm/glm.hpp"

#include <algorithm>

bool ZoneData::isZoneContained(const ZoneData &inner, const ZoneData &outer) {
    return glm::all(glm::greaterThanEqual(inner.min, outer.min)) &&
            glm::all(glm::lessThanEqual(inner.max, outer.max));
}

bool ZoneData::containsPoint(const glm::vec3 &point) const {
    return glm::all(glm::greaterThanEqual(point, min)) &&
            glm::all(glm::lessThanEqual(point, max));
}

ZoneData *ZoneData::findLeafAtPoint(const glm::vec3 &point) {
    for (ZoneData* child : children_) {
        auto descendent = child->findLeafAtPoint(point);
        if (descendent) {
            return descendent;
        }
    }
    return containsPoint(point) ? this : nullptr;
}

bool ZoneData::insertZone(ZoneData &inner) {
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
