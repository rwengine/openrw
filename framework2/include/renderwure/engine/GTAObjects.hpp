#pragma once
#ifndef _GTAOBJECTS_HPP_
#define _GTAOBJECTS_HPP_

#include <renderwure/loaders/LoaderIDE.hpp>
#include <renderwure/loaders/LoaderIPL.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>

/**
 * @struct GTAObject
 *  Stores references to the Object data and the instance
 */
struct GTAInstance {
    LoaderIPLInstance instance;
    std::shared_ptr<LoaderIDE::OBJS_t> object;
};

/**
 * @brief The GTACharacter struct
 * Stores data relating to an instance of a "pedestrian".
 */
struct GTACharacter {
    std::shared_ptr<LoaderIDE::PEDS_t> ped;
    glm::vec3 position;
    glm::quat rotation;
};

/**
 * @class GTAVehicle
 *  Stores references to the vehicle data and the instance
 */
struct GTAVehicle {
    std::shared_ptr<LoaderIDE::CARS_t> vehicle; /// Vehicle type
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 colourPrimary;
    glm::vec3 colourSecondary;
};



#endif // GTAOBJECTS_HPP
