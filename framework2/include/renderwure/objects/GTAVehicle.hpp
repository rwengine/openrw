#pragma once
#ifndef _GTAVEHICLE_HPP_
#define _GTAVEHICLE_HPP_
#include <renderwure/engine/GTAObject.hpp>
#include <bullet/btBulletDynamicsCommon.h>
#include <renderwure/objects/VehicleInfo.hpp>

/**
 * @class GTAVehicle
 *  Stores references to the vehicle data and the instance
 */
struct GTAVehicle : public GTAObject
{
	std::shared_ptr<LoaderIDE::CARS_t> vehicle; /// Vehicle type
	VehicleInfo info;
	glm::vec3 colourPrimary;
	glm::vec3 colourSecondary;

	btRigidBody* physBody;
	btVehicleRaycaster* physRaycaster;
	btRaycastVehicle* physVehicle;

	GTAVehicle(GTAEngine* engine,
			   const glm::vec3& pos,
			   const glm::quat& rot,
			   Model* model,
			   std::shared_ptr<LoaderIDE::CARS_t> veh,
			   const VehicleInfo& info,
			   const glm::vec3& prim,
			   const glm::vec3& sec);

	glm::vec3 getPosition() const;

	glm::quat getRotation() const;

	Type type() { return Vehicle; }

	void tick(float dt);
};

#endif
