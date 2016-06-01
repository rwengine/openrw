#ifndef RWENGINE_VEHICLEDYNAMICS_HPP
#define RWENGINE_VEHICLEDYNAMICS_HPP

#include <glm/glm.hpp>

#include <vector>

class VehicleObject;
class btRigidBody;

/**
 * @brief The VehicleDynamics class
 *
 * implements vehicle physics, using raycasts.
 *
 */
class VehicleDynamics
{
public:

	struct WheelInfo
	{
		/// Position of the wheel relative to the chassis
		glm::vec3 position;
		/// Axle direction
		glm::vec3 axle;
		/// Wheel radius
		float radius;
		/// How much engine power this wheel can consume [0-1]
		float maxPowerFrac;
		/// Vertical displacement
		float displacement;
		/// Current steering angle
		float steerAngle;
		/// Rotation around the axle axis
		float rotation;
		/// Angular velocity
		float rotationVelocity;
		/// Ray length
		float rayLength;
		/// Is wheel touching the ground
		bool isOnGround;
		/// Force that was applied to the vehicle
		float suspensionForce;
		// Applied suspension force
		glm::vec3 suspensionForceWS;
		// Applied traction force
		glm::vec3 tractionForceWS;

		WheelInfo(const glm::vec3& pos, const glm::vec3& axle, float radius, float powerFrac)
			: position(pos)
			, axle(axle)
			, radius(radius)
			, maxPowerFrac(powerFrac)
			, displacement(0.f)
			, steerAngle(0.f)
			, rotation(0.f)
			, rotationVelocity(0.f)
			, rayLength(0.f)
			, isOnGround(false)
			, suspensionForce(0.f)
		{}
	};

	VehicleDynamics(VehicleObject *vehicle)
		: m_vehicle(vehicle)
		, m_engineForce(0.f)
		, m_breakForce(0.f)
	{ }

	void addWheel(const glm::vec3& position, const glm::vec3& axle, float radius, float power)
	{
		m_wheels.emplace_back(position, axle, radius, power);
	}

	/**
	 * @brief tickPhysics apply physics.
	 * @param dt
	 */
	void tickPhysics(float dt);

	const std::vector<WheelInfo>& getWheels() const
	{
		return m_wheels;
	}

	void setEngineForce(float force)
	{
		m_engineForce = force;
	}

	void setBreakForce(float force)
	{
		m_breakForce = force;
	}

	void setWheelAngle(int wheel, float angle)
	{
		m_wheels[wheel].steerAngle = angle;
	}

private:
	VehicleObject *m_vehicle;
	std::vector<WheelInfo> m_wheels;
	float m_engineForce;
	float m_breakForce;

	bool raycastWheel(const WheelInfo& wheel, float& distFrac, glm::vec3& worldHit, const btRigidBody*& body);
};

#endif
