#pragma once
#ifndef _GTAOBJECTS_HPP_
#define _GTAOBJECTS_HPP_

#include <renderwure/engine/GTATypes.hpp>
#include <renderwure/loaders/LoaderIDE.hpp>
#include <renderwure/loaders/LoaderIPL.hpp>
#include <glm/gtc/quaternion.hpp>
#include <bullet/BulletDynamics/Character/btKinematicCharacterController.h>
#include <bullet/btBulletCollisionCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <memory>

class GTAAIController;
class Model;
class Animation;

class GTAEngine;

/**
 * @brief The GTAObject struct
 * Stores data that is relevant to all types of objects.
 */
struct GTAObject
{
    glm::vec3 position;
    glm::quat rotation;

    Model* model; /// Cached pointer to Object's Model.
    
    GTAEngine* engine;

    Animation* animation; /// The currently playing animation.
    float animtime; /// The current time in the animation.
    float lastAnimtime;
    glm::vec3 rootPosition; /// Where the object was at the start of animation
    glm::vec3 lastRootPosition;

    GTAObject(GTAEngine* engine, const glm::vec3& pos, const glm::quat& rot, Model* model)
        : position(pos), rotation(rot), model(model), engine(engine), animation(nullptr), animtime(0.f), lastAnimtime(0.f) {}

    enum Type
    {
        Instance,
        Character,
        Vehicle
    };

    virtual Type type() = 0;

    /**
     * @brief updateBones Updates frame matrices
     * Updates the internal frame matrices, taking into account the current animation.
     */
    void updateFrames();
	
	virtual void setPosition(const glm::vec3& pos);
	
	virtual glm::vec3 getPosition() const;
};

/**
 * @struct GTAInstance
 *  Stores references to the Object data and the instance
 */
struct GTAInstance : public GTAObject
{
    glm::vec3 scale;
    LoaderIPLInstance instance;
    std::shared_ptr<LoaderIDE::OBJS_t> object;

    GTAInstance(GTAEngine* engine, const glm::vec3& pos, const glm::quat& rot, Model* model, const glm::vec3& scale, LoaderIPLInstance inst, std::shared_ptr<LoaderIDE::OBJS_t> obj)
        : GTAObject(engine, pos, rot, model), scale(scale), instance(inst), object(obj) {}

    Type type() { return Instance; }
};

/**
 * @brief The GTACharacter struct
 * Stores data relating to an instance of a "pedestrian".
 */
struct GTACharacter : public GTAObject
{
    enum Activity {
        None,
        Idle,
        Walk,
        Run,
        Crouch
    };

    std::shared_ptr<LoaderIDE::PEDS_t> ped;

    btKinematicCharacterController* physCharacter;
    btPairCachingGhostObject* physObject;
    btBoxShape* physShape;
	
	GTAAIController* controller;

    /**
     * @brief GTACharacter Constructs a Character
     * @param pos
     * @param rot
     * @param model
     * @param ped PEDS_t struct to use.
     */
    GTACharacter(GTAEngine* engine, const glm::vec3& pos, const glm::quat& rot, Model* model, std::shared_ptr<LoaderIDE::PEDS_t> ped);

    Type type() { return Character; }

    Activity currentActivity;

    void changeAction(Activity newAction);

    /**
     * @brief updateCharacter updates internall bullet Character.
     */
    void updateCharacter();

    /**
     * @brief updateAnimation updates animation parameters
     */
    void updateAnimation(float dt);
	
	virtual void setPosition(const glm::vec3& pos);
	
	virtual glm::vec3 getPosition() const;
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

    GTAVehicle(GTAEngine* engine, const glm::vec3& pos, const glm::quat& rot, Model* model, std::shared_ptr<LoaderIDE::CARS_t> veh, const glm::vec3& prim, const glm::vec3& sec)
        : GTAObject(engine, pos, rot, model), vehicle(veh), colourPrimary(prim), colourSecondary(sec) {}

    Type type() { return Vehicle; }
};



#endif // GTAOBJECTS_HPP
