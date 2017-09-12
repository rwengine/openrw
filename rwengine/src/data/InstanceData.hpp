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
    /**
     * Constructor
     */
    InstanceData(int _id, std::string _model, glm::vec3 _pos, glm::vec3 _scale,  glm::quat _rot)
        : id(_id)
        , model(_model)
        , pos(_pos)
        , scale(_scale)
        , rot(_rot){
    }

};

#endif
