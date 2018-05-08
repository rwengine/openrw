#ifndef _RWENGINE_INSTANCEDATA_HPP_
#define _RWENGINE_INSTANCEDATA_HPP_
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
     template <class String>
    InstanceData(int _id, String&& _model, glm::vec3 _pos, glm::vec3 _scale,  glm::quat _rot)
        : id(_id)
        , model(std::forward<String>(_model))
        , pos(_pos)
        , scale(_scale)
        , rot(_rot){
    }

};

#endif
