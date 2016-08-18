#pragma once
#ifndef _CUTSCENEDATA_HPP_
#define _CUTSCENEDATA_HPP_
#include <glm/glm.hpp>
#include <map>
#include <vector>
#include <string>

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
  glm::vec3 sceneOffset;

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
  std::map<float, glm::vec3> target;  /// Rotation is around the direction to this vector

  float duration{0.f};

  glm::vec3 getPositionAt(float time)
  {
    glm::vec3 p = position.end()->second;
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

  glm::vec3 getTargetAt(float time)
  {
    glm::vec3 p = position.end()->second;
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

  float getZoomAt(float time)
  {
    float r = zoom.end()->second;
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

  float getRotationAt(float time)
  {
    float r = rotation.end()->second;
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
};

struct CutsceneData {
  CutsceneMetadata meta;
  CutsceneTracks tracks;
};

#endif
