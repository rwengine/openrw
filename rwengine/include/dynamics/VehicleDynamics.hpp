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
		/// Vertical displacement
		float displacement;
		/// Rotation around the axle axis
		float rotation;
		/// Angular velocity
		float rotationVelocity;
		/// Ray length
		float rayLength;

		WheelInfo(const glm::vec3& pos, const glm::vec3& axle, float radius)
			: position(pos)
			, axle(axle)
			, radius(radius)
			, displacement(0.f)
			, rotation(0.f)
			, rotationVelocity(0.f)
			, rayLength(0.f)
		{}
	};

	VehicleDynamics(VehicleObject *vehicle)
		: m_vehicle(vehicle)
	{ }

	void addWheel(const glm::vec3& position, const glm::vec3& axle, float radius)
	{
		m_wheels.emplace_back(position, axle, radius);
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

private:
	VehicleObject *m_vehicle;
	std::vector<WheelInfo> m_wheels;

	bool raycastWheel(const WheelInfo& wheel, float& distFrac, glm::vec3& worldHit, const btRigidBody*& body);
};

#endif
