#pragma once
#ifndef _GTAVEHICLE_HPP_
#define _GTAVEHICLE_HPP_
#include <renderwure/engine/GTAObject.hpp>
#include <bullet/btBulletDynamicsCommon.h>

/**
 * @class GTAVehicle
 *  Stores references to the vehicle data and the instance
 */
struct GTAVehicle : public GTAObject
{
	std::shared_ptr<LoaderIDE::CARS_t> vehicle; /// Vehicle type
	glm::vec3 colourPrimary;
	glm::vec3 colourSecondary;

	btRigidBody* physBody;

	GTAVehicle(GTAEngine* engine, const glm::vec3& pos, const glm::quat& rot, Model* model, std::shared_ptr<LoaderIDE::CARS_t> veh, const glm::vec3& prim, const glm::vec3& sec);

	glm::vec3 getPosition() const;

	glm::quat getRotation() const;

	Type type() { return Vehicle; }
};

#endif
