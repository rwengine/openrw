#pragma once
#ifndef _VEHICLEOBJECT_HPP_
#define _VEHICLEOBJECT_HPP_
#include <objects/GameObject.hpp>
#include <map>
#include <objects/VehicleInfo.hpp>

class CollisionInstance;
class btVehicleRaycaster;
class btRaycastVehicle;
class btRigidBody;
class btHingeConstraint;
class btTransform;

/**
 * @class VehicleObject
 * Implements Vehicle behaviours.
 */
class VehicleObject : public GameObject
{
private:
	float steerAngle;
	float throttle;
	float brake;
	bool handbrake;
public:

	VehicleDataHandle vehicle;
	VehicleInfoHandle info;
	glm::u8vec3 colourPrimary;
	glm::u8vec3 colourSecondary;
	
	std::map<size_t, GameObject*> seatOccupants;

	CollisionInstance* collision;
	btVehicleRaycaster* physRaycaster;
	btRaycastVehicle* physVehicle;
	
	struct Part
	{
		ModelFrame* dummy;
		ModelFrame* normal;
		ModelFrame* damaged;
		btRigidBody* body;
		btHingeConstraint* constraint;
		bool moveToAngle;
		float targetAngle;
		float openAngle;
		float closedAngle;
	};
	
	std::map<std::string, Part> dynamicParts;

	VehicleObject(GameWorld* engine, const glm::vec3& pos, const glm::quat& rot, const ModelRef& model, VehicleDataHandle data, VehicleInfoHandle info, const glm::u8vec3& prim, const glm::u8vec3& sec);
	
	virtual ~VehicleObject();
	
	void setPosition(const glm::vec3& pos);

	glm::vec3 getPosition() const;

	void setRotation(const glm::quat &orientation);

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

	void tickPhysics(float dt);
	
	bool isFlipped() const;

	float getVelocity() const;
	
	void ejectAll();
	
	GameObject* getOccupant(size_t seat);
	
	void setOccupant(size_t seat, GameObject* occupant);

	/**
	 * @brief canOccupantExit
	 * @return true if the vehicle is currently exitable
	 */
	bool canOccupantExit() const;

	/**
	 * @brief isOccupantDriver
	 * @param seat
	 * @return True if the given seat is the driver's seat.
	 */
	bool isOccupantDriver(size_t seat) const;

	glm::vec3 getSeatEntryPosition(size_t seat) const {
		auto pos = info->seats[seat].offset;
		pos -= glm::vec3(glm::sign(pos.x) * -0.81756252f, 0.34800607f, -0.486281008f);
		return getPosition() + getRotation() * pos;
	}
	
	Part* getSeatEntryDoor(size_t seat);
	
    virtual bool takeDamage(const DamageInfo& damage);

	enum FrameState {
		OK,
		DAM,
		BROKEN
	};

	void setPartState(Part* part, FrameState state);

	void setPartLocked(Part* part, bool locked);
	
	void setPartTarget(Part* part, bool enable, float target);

	Part* getPart(const std::string& name);

	void applyWaterFloat(const glm::vec3& relPt);

	void setPrimaryColour(uint8_t color);
	void setSecondaryColour(uint8_t color);

private:

	void registerPart(ModelFrame* mf);
	void createObjectHinge(btTransform &local, Part* part);
	void destroyObjectHinge(Part* part);
};

#endif
