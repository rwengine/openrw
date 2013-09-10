#pragma once
#ifndef _GTAOBJECTS_HPP_
#define _GTAOBJECTS_HPP_

#include <renderwure/engine/GTATypes.hpp>
#include <renderwure/loaders/LoaderIDE.hpp>
#include <renderwure/loaders/LoaderIPL.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>

class GTAAIController;
class Model;
class Animator;

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

	Animator* animator; /// Object's animator.

    GTAObject(GTAEngine* engine, const glm::vec3& pos, const glm::quat& rot, Model* model)
		: position(pos), rotation(rot), model(model), engine(engine), animator(nullptr) {}

    enum Type
    {
        Instance,
        Character,
        Vehicle
    };

    virtual Type type() = 0;
	
	virtual void setPosition(const glm::vec3& pos);
	
	virtual glm::vec3 getPosition() const;

	virtual glm::quat getRotation() const;
};

#endif // GTAOBJECTS_HPP
