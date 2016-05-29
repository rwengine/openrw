#include <dynamics/VehicleDynamics.hpp>
#include <dynamics/CollisionInstance.hpp>
#include <dynamics/RaycastCallbacks.hpp>
#include <objects/VehicleObject.hpp>
#include <engine/GameWorld.hpp>

void VehicleDynamics::tickPhysics(float dt)
{
	float distFrac;
	glm::vec3 worldHit;
	const btRigidBody* body;
	const float upperLimit = m_vehicle->info->handling.suspensionUpperLimit;
	const float restLength = upperLimit -
			m_vehicle->info->handling.suspensionLowerLimit;
	// ??
	const float springForce = (m_vehicle->info->handling.suspensionForce / restLength) * 1000.f;
	const float springDamp = (m_vehicle->info->handling.suspensionDamping / restLength) * 10000.f;
	auto bulletBody = m_vehicle->collision->getBulletBody();

	// Downforce = 16677
	// Force per wheel ~= 4169.25

	for(auto& wheel : m_wheels)
	{
		// Calculate the ray length to ensure this ray can't fall through the ground.
		wheel.rayLength = restLength + wheel.radius;
		if (raycastWheel(wheel, distFrac, worldHit, body))
		{
			float distance = std::min((distFrac * wheel.rayLength) - wheel.radius,
									  restLength);
			// The center of the wheel must be radius above the end point.
			wheel.displacement = glm::min(0.f, -distance);
			float force = std::max(0.f, springForce * (restLength - distance)/restLength);
			printf("force: %f displacement: %f distFrac %f\n", force, wheel.displacement, (restLength - distance)/restLength);
			// Move force location into local space.
			auto ws = m_vehicle->getRotation() * wheel.position;
			auto btws = btVector3(ws.x, ws.y, ws.z);
			auto pointVel = bulletBody->getLinearVelocity();
			force += -pointVel.z() * springDamp;
#if 1
			bulletBody->applyImpulse(
						btVector3(0.f, 0.f, force * dt),
						btws);
#else
			bulletBody->applyForce(
						btVector3(0.f, 0.f, force),
						btws);
#endif
		}
		else
		{
			wheel.displacement = -restLength;
		}
	}

}

bool VehicleDynamics::raycastWheel(const VehicleDynamics::WheelInfo& wheel, float& distFrac, glm::vec3& worldHit, const btRigidBody* &body)
{
	const float& handling_upperlimit = m_vehicle->info->handling.suspensionUpperLimit;

	auto bulletbody = m_vehicle->collision->getBulletBody();
	glm::vec3 wheelStartWorldSpace = m_vehicle->getPosition()
			+ m_vehicle->getRotation()
			* (wheel.position + glm::vec3(0.f, 0.f, handling_upperlimit));
	glm::vec3 wheelEndWorldSpace = m_vehicle->getPosition()
			+ m_vehicle->getRotation()
			* (wheel.position + glm::vec3(0.f, 0.f, handling_upperlimit - wheel.rayLength));
	btVector3 from(wheelStartWorldSpace.x, wheelStartWorldSpace.y, wheelStartWorldSpace.z);
	btVector3 to(wheelEndWorldSpace.x, wheelEndWorldSpace.y, wheelEndWorldSpace.z);

	ClosestNotMeRayResultCallback rayCallback( bulletbody, from, to );

	auto world = m_vehicle->engine;
	world->dynamicsWorld->rayTest(from, to, rayCallback);

	if (rayCallback.hasHit()) {
		const btRigidBody* raybody = btRigidBody::upcast( rayCallback.m_collisionObject );

		if( raybody && raybody->hasContactResponse() ) {
			worldHit.x = rayCallback.m_hitPointWorld.x();
			worldHit.y = rayCallback.m_hitPointWorld.y();
			worldHit.z = rayCallback.m_hitPointWorld.z();

			//result.m_hitNormalInWorld = rayCallback.m_hitNormalWorld; //??
			//result.m_hitNormalInWorld.normalize();

			distFrac = rayCallback.m_closestHitFraction;
			body = raybody;
			return true;
		}
	}
	else {
		distFrac = 1.f;
		body = nullptr;
	}
	return false;
}
