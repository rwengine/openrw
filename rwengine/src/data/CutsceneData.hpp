#ifndef _RWENGINE_CUTSCENEDATA_HPP_
#define _RWENGINE_CUTSCENEDATA_HPP_

#include <glm/vec3.hpp>

#include <map>
#include <string>
#include <vector>

/**
 * @brief Stores data from .CUT files
 */
struct CutsceneMetadata {
    struct ModelEntry {
        std::string model;
        std::string animation;
    };
    struct TextEntry {
        float length;
        std::string gxt;
    };

    std::string name;

    /// The origin for coordinates in the cutscene
    glm::vec3 sceneOffset{};

    std::vector<ModelEntry> models;
    std::map<float, TextEntry> texts;
};

/**
 * @brief Stores the Camera animation data from .DAT files
 */
struct CutsceneTracks {
    std::map<float, float> zoom;
    std::map<float, float> rotation;
    std::map<float, glm::vec3> position;
    std::map<float, glm::vec3> target;
    /* Rotation is angle around the target vector */

    float duration{0.f};

    glm::vec3 getPositionAt(float time) const;

    glm::vec3 getTargetAt(float time) const;

    float getZoomAt(float time) const;

    float getRotationAt(float time) const;
};

struct CutsceneData {
    CutsceneMetadata meta;
    CutsceneTracks tracks;
};

#endif
