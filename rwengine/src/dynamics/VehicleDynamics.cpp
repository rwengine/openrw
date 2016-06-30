#include <dynamics/VehicleDynamics.hpp>
#include <dynamics/CollisionInstance.hpp>
#include <dynamics/RaycastCallbacks.hpp>
#include <objects/VehicleObject.hpp>
#include <engine/GameWorld.hpp>

void VehicleDynamics::tickPhysics(float dt)
{
	float distFrac;
	glm::vec3 worldHit;
	glm::vec3 worldNormal;
	const btRigidBody* body;
	const float upperLimit = m_vehicle->info->handling.suspensionUpperLimit;
	const float restLength = upperLimit -
			m_vehicle->info->handling.suspensionLowerLimit;
	const float vehicleMass = m_vehicle->info->handling.mass;

	const float suspensionMod =
	    m_vehicle->info->handling.suspensionForce / restLength;
	const float dampingMod =
	    m_vehicle->info->handling.suspensionDamping / restLength;
	const float springForce = suspensionMod * vehicleMass * 10.f;
	const float springDamp = dampingMod * vehicleMass * 10.f;
	auto bulletBody = m_vehicle->collision->getBulletBody();

	auto r = m_vehicle->getRotation();

	// Handle suspension, determine which wheels are on the ground
	float totalPower = 0.f;
	const float vehicleSuspensionBias = m_vehicle->info->handling.suspensionBias;
	for(auto& wheel : m_wheels)
	{
		// Calculate the ray length to ensure this ray can't fall through the ground.
		wheel.rayLength = restLength + wheel.radius;
		wheel.isOnGround =
		    raycastWheel(wheel, distFrac, worldHit, worldNormal, body);
		totalPower += wheel.maxPowerFrac;
		if (wheel.isOnGround)
		{
			float distance = std::min((distFrac * wheel.rayLength) - wheel.radius,
									  restLength);
			float suspensionBias = (wheel.position.y > 0.f) ? vehicleSuspensionBias : 1.f - vehicleSuspensionBias;

			// The center of the wheel must be radius above the end point.
			wheel.displacement = glm::min(0.f, -distance);
			float force = std::max(0.f, springForce * (restLength - distance)/restLength);

			// Calculate the Normal force
			float norm = glm::dot(glm::vec3(0.f, 0.f, 1.f), worldNormal);
			glm::vec3 normalForce = norm * worldNormal * force;

			// Move force location into local space.
			auto ws = m_vehicle->getRotation() * wheel.position;
			auto btws = btVector3(ws.x, ws.y, ws.z);

			auto pointVel = bulletBody->getVelocityInLocalPoint(btws);
			auto localVelocity = glm::inverse(r) * glm::vec3(pointVel.x(), pointVel.y(), pointVel.z());

			float damp = -localVelocity.z * springDamp;
			auto f = normalForce * suspensionBias + glm::vec3(0.f, 0.f, damp);
			wheel.suspensionForce = force;
			wheel.suspensionForceWS = f;
			bulletBody->applyForce(
						btVector3(f.x, f.y, f.z),
						btws);
		}
		else
		{
			wheel.displacement = -restLength;
			wheel.suspensionForceWS = glm::vec3(0.f, 0.f, 0.f);
		}
	}

	const float vehicleTractionBias = m_vehicle->info->handling.tractionBias;
	const float vehicleTractionMulti = m_vehicle->info->handling.tractionMulti;
	const float vehicleTractionLoss = m_vehicle->info->handling.tractionLoss;
	const float vehicleBrakeBias = m_vehicle->info->handling.brakeBias;
	// Handle traction & engine force
	for(auto& wheel : m_wheels)
	{
		if (wheel.isOnGround)
		{
			float tractionBias = (wheel.position.y > 0.f) ? vehicleTractionBias : 1.f - vehicleTractionBias;
			float brakeBias = (wheel.position.y > 0.f) ? vehicleBrakeBias : 1.f - vehicleBrakeBias;
			auto maxImpulse = wheel.suspensionForce
					* glm::vec2(vehicleTractionMulti, vehicleTractionLoss) * tractionBias * 2.f;

			auto ws = m_vehicle->getRotation() * wheel.position;
			auto btws = btVector3(ws.x, ws.y, ws.z);

			// Assume power is always distributed equally, some kind of magic differential.
			float force = (wheel.maxPowerFrac * m_engineForce)/totalPower;
			auto f = glm::vec3(0.f, -force, 0.f);
			// Rotate force to face wheel's steering direction
			glm::quat wheelRotation(wheel.steerAngle, glm::vec3(0.f, 0.f,-1.f));
			auto wr = r * wheelRotation;
			auto pointVel = bulletBody->getVelocityInLocalPoint(btws);
			auto localVelocity = glm::inverse(wr) * glm::vec3(pointVel.x(), pointVel.y(), pointVel.z());

			float mu = 0.8f;
			float brakePower = m_breakForce * m_vehicle->info->handling.brakeDeceleration * -localVelocity.y * brakeBias;
			auto impulse = glm::vec2(-localVelocity.x, brakePower) * vehicleMass * mu;
			auto impulseSq = impulse * impulse;
			auto maxImpulseSq = maxImpulse * maxImpulse;

			// +x is to the right hand side
			if (impulseSq.x > maxImpulseSq.x)
			{
				impulse.x = glm::sign(impulse.x) * maxImpulse.x;
				// skidding =  yes
			}
			if (impulseSq.y > maxImpulseSq.y)
			{
				impulse.y = glm::sign(impulse.y) * maxImpulse.y;
				// slipping = also yes
			}

			f = wr * f;
			f += wr * glm::vec3(impulse, 0.f);
			wheel.tractionForceWS = f;
			bulletBody->applyForce(
						btVector3(f.x, f.y, f.z),
						btws);
			float radiusVel = localVelocity.y * dt;

			wheel.rotation += 1.f/((glm::pi<float>()*2*wheel.radius)/(glm::pi<float>() * 2 * radiusVel));
		}
		else
		{
			wheel.tractionForceWS = glm::vec3(0.f, 0.f, 0.f);
		}
	}
}

bool VehicleDynamics::raycastWheel(const VehicleDynamics::WheelInfo& wheel,
                                   float& distFrac,
                                   glm::vec3& worldHit,
                                   glm::vec3& worldNormal,
                                   const btRigidBody*& body)
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

			worldNormal.x = rayCallback.m_hitNormalWorld.x();
			worldNormal.y = rayCallback.m_hitNormalWorld.y();
			worldNormal.z = rayCallback.m_hitNormalWorld.z();

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
