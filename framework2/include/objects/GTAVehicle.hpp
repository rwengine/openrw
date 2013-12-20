#pragma once
#ifndef _GTAVEHICLE_HPP_
#define _GTAVEHICLE_HPP_
#include <engine/GTAObject.hpp>
#include <bullet/btBulletDynamicsCommon.h>
#include <map>
#include <objects/VehicleInfo.hpp>

/**
 * @class GTAVehicle
 *  Stores references to the vehicle data and the instance
 */
struct GTAVehicle : public GTAObject
{
private:
	float steerAngle;
	float throttle;
	float brake;
	bool handbrake;
public:

	std::shared_ptr<CarData> vehicle;
	VehicleInfo info;
	glm::vec3 colourPrimary;
	glm::vec3 colourSecondary;
	
	std::map<size_t, GTAObject*> seatOccupants;

	btRigidBody* physBody;
	btVehicleRaycaster* physRaycaster;
	btRaycastVehicle* physVehicle;

	GTAVehicle(GTAEngine* engine,
			   const glm::vec3& pos,
			   const glm::quat& rot,
			   Model* model,
			   std::shared_ptr<CarData> data,
			   const VehicleInfo& info,
			   const glm::vec3& prim,
			   const glm::vec3& sec);
	
    virtual ~GTAVehicle();
	
	void setPosition(const glm::vec3& pos);

	glm::vec3 getPosition() const;

	glm::quat getRotation() const;

	Type type() { return Vehicle; }

	void setSteeringAngle(float);

	float getSteeringAngle() const;

	void setThrottle(float);

	float getThrottle() const;

	void setBraking(float);

	float getBraking() const;

	void setHandbraking(bool);

	bool getHandbraking() const;

	void tick(float dt);
	
	void ejectAll();
	
	GTAObject* getOccupant(size_t seat);
	
	void setOccupant(size_t seat, GTAObject* occupant);
	
    virtual bool takeDamage(const DamageInfo& damage);
};

#endif
