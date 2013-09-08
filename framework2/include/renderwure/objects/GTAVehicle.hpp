#pragma once
#ifndef _GTAVEHICLE_HPP_
#define _GTAVEHICLE_HPP_
#include <renderwure/engine/GTAObject.hpp>

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

#endif
