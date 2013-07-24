#pragma once
#ifndef _GTAOBJECTS_HPP_
#define _GTAOBJECTS_HPP_

#include <renderwure/loaders/LoaderIDE.hpp>
#include <renderwure/loaders/LoaderIPL.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>

class Model;

/**
 * @brief The GTAObject struct
 * Stores data that is relevant to all types of objects.
 */
struct GTAObject
{
    glm::vec3 position;
    glm::quat rotation;

    Model* model; /// Cached pointer to Object's Model.

    GTAObject(const glm::vec3& pos, const glm::quat& rot, Model* model)
        : position(pos), rotation(rot), model(model) {}

    enum Type
    {
        Instance,
        Character,
        Vehicle
    };

    virtual Type type() = 0;
};

/**
 * @struct GTAObject
 *  Stores references to the Object data and the instance
 */
struct GTAInstance : public GTAObject
{
    glm::vec3 scale;
    LoaderIPLInstance instance;
    std::shared_ptr<LoaderIDE::OBJS_t> object;

    GTAInstance(const glm::vec3& pos, const glm::quat& rot, Model* model, const glm::vec3& scale, LoaderIPLInstance inst, std::shared_ptr<LoaderIDE::OBJS_t> obj)
        : GTAObject(pos, rot, model), scale(scale), instance(inst), object(obj) {}

    Type type() { return Instance; }
};

/**
 * @brief The GTACharacter struct
 * Stores data relating to an instance of a "pedestrian".
 */
struct GTACharacter : public GTAObject
{
    std::shared_ptr<LoaderIDE::PEDS_t> ped;

    GTACharacter(const glm::vec3& pos, const glm::quat& rot, Model* model, std::shared_ptr<LoaderIDE::PEDS_t> ped)
        : GTAObject(pos, rot, model), ped(ped) {}

    Type type() { return Character; }
};

/**
 * @class GTAVehicle
 *  Stores references to the vehicle data and the instance
 */
struct GTAVehicle : public GTAObject
{
    std::shared_ptr<LoaderIDE::CARS_t> vehicle; /// Vehicle type
    glm::vec3 colourPrimary;
    glm::vec3 colourSecondary;

    GTAVehicle(const glm::vec3& pos, const glm::quat& rot, Model* model, std::shared_ptr<LoaderIDE::CARS_t> veh, const glm::vec3& prim, const glm::vec3& sec)
        : GTAObject(pos, rot, model), vehicle(veh), colourPrimary(prim), colourSecondary(sec) {}

    Type type() { return Vehicle; }
};



#endif // GTAOBJECTS_HPP
