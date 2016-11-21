#pragma once
#ifndef __INSTANCEDATA_HPP__
#define __INSTANCEDATA_HPP__
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>

struct InstanceData {
    /**
     * ID of the object this instance
     */
    int id;
    /**
     * Model name
     */
    std::string model;
    /**
     * Instance position
     */
    glm::vec3 pos;
    /**
     * Instance scaleX
     */
    glm::vec3 scale;
    /**
     * Instance rotation
     */
    glm::quat rot;
};

#endif