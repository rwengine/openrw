#include "data/CutsceneData.hpp"

#include <glm/glm.hpp>

glm::vec3 CutsceneTracks::getPositionAt(float time) const {
    glm::vec3 p = position.rbegin()->second;
    for (auto it = position.begin(); it != position.end(); ++it) {
        if (it->first <= time) {
            auto a = it->second;
            auto b = it->second;
            auto nextIt = it;
            float t = it->first;
            if (++nextIt != position.end()) {
                b = nextIt->second;
                t = nextIt->first;
            }
            float tdiff = t - it->first;
            p = b;
            if (tdiff > 0.f) {
                float fac = (time - it->first) / tdiff;
                p = glm::mix(a, b, fac);
            }
        }
    }
    return p;
}

glm::vec3 CutsceneTracks::getTargetAt(float time) const {
    glm::vec3 p = position.rbegin()->second;
    for (auto it = target.begin(); it != target.end(); ++it) {
        if (it->first <= time) {
            auto a = it->second;
            auto b = it->second;
            auto nextIt = it;
            float t = it->first;
            if (++nextIt != target.end()) {
                b = nextIt->second;
                t = nextIt->first;
            }
            float tdiff = t - it->first;
            p = b;
            if (tdiff > 0.f) {
                float fac = (time - it->first) / tdiff;
                p = glm::mix(a, b, fac);
            }
        }
    }
    return p;
}

float CutsceneTracks::getZoomAt(float time) const {
    float r = zoom.rbegin()->second;
    for (auto it = zoom.begin(); it != zoom.end(); ++it) {
        if (it->first <= time) {
            auto a = it->second;
            auto b = it->second;
            auto nextIt = it;
            float t = it->first;
            if (++nextIt != zoom.end()) {
                b = nextIt->second;
                t = nextIt->first;
            }
            float tdiff = t - it->first;
            r = b;
            if (tdiff > 0.f) {
                float fac = (time - it->first) / tdiff;
                r = glm::mix(a, b, fac);
            }
        }
    }
    return r;
}

float CutsceneTracks::getRotationAt(float time) const {
    float r = rotation.rbegin()->second;
    for (auto it = rotation.begin(); it != rotation.end(); ++it) {
        if (it->first <= time) {
            auto a = it->second;
            auto b = it->second;
            auto nextIt = it;
            float t = it->first;
            if (++nextIt != rotation.end()) {
                b = nextIt->second;
                t = nextIt->first;
            }
            float tdiff = t - it->first;
            r = b;
            if (tdiff > 0.f) {
                float fac = (time - it->first) / tdiff;
                r = glm::mix(a, b, fac);
            }
        }
    }
    return r;
}
