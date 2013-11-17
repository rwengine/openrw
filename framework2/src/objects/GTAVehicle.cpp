#include <renderwure/objects/GTAVehicle.hpp>
#include <renderwure/engine/GTAEngine.hpp>
#include <BulletDynamics/Vehicle/btRaycastVehicle.h>

GTAVehicle::GTAVehicle(GTAEngine* engine, const glm::vec3& pos, const glm::quat& rot, Model* model, std::shared_ptr<LoaderIDE::CARS_t> veh, const VehicleInfo& info, const glm::vec3& prim, const glm::vec3& sec)
	: GTAObject(engine, pos, rot, model),
	  steerAngle(0.f), throttle(0.f), brake(0.f), handbrake(false),
	  vehicle(veh), info(info), colourPrimary(prim), colourSecondary(sec), physBody(nullptr), physVehicle(nullptr)
{
	if(! veh->modelName.empty()) {
		auto phyit = engine->gameData.collisions.find(veh->modelName);
		if( phyit != engine->gameData.collisions.end()) {
			btCompoundShape* cmpShape = new btCompoundShape;
			btDefaultMotionState* msta = new btDefaultMotionState;
			msta->setWorldTransform(btTransform(
				btQuaternion(
					rot.x, rot.y, rot.z, rot.w
				),
				btVector3(
					pos.x, pos.y, pos.z
				)
			));
			CollisionModel& physInst = *phyit->second.get();

			btVector3 com(info.handling.centerOfMass.x, info.handling.centerOfMass.y, info.handling.centerOfMass.z);

			// Boxes
			for( size_t i = 0; i < physInst.boxes.size(); ++i ) {
				auto& box = physInst.boxes[i];
				auto size = (box.max - box.min) / 2.f;
				auto mid = (box.min + box.max) / 2.f;
				btCollisionShape* bshape = new btBoxShape( btVector3(size.x, size.y, size.z)  );
				btTransform t; t.setIdentity();
				t.setOrigin(btVector3(mid.x, mid.y, mid.z) + com);
				cmpShape->addChildShape(t, bshape);
			}

			// Spheres
			for( size_t i = 0; i < physInst.spheres.size(); ++i ) {
				auto& sphere = physInst.spheres[i];
				btCollisionShape* sshape = new btSphereShape(sphere.radius);
				btTransform t; t.setIdentity();
				t.setOrigin(btVector3(sphere.center.x, sphere.center.y, sphere.center.z) + com);
				cmpShape->addChildShape(t, sshape);
			}

			if( physInst.vertices.size() > 0 && physInst.indices.size() >= 3 ) {
				btTriangleIndexVertexArray* vertarray = new btTriangleIndexVertexArray(
							physInst.indices.size()/3,
							(int*) physInst.indices.data(),
							sizeof(uint32_t)*3,
							physInst.vertices.size(),
							&(physInst.vertices[0].x),
							sizeof(glm::vec3));
				btBvhTriangleMeshShape* trishape = new btBvhTriangleMeshShape(vertarray, false);
				btTransform t; t.setIdentity();
				cmpShape->addChildShape(t,trishape);
			}

			btVector3 inertia(0,0,0);
			cmpShape->calculateLocalInertia(info.handling.mass, inertia);

			btRigidBody::btRigidBodyConstructionInfo rginfo(info.handling.mass, msta, cmpShape, inertia);

			physBody = new btRigidBody(rginfo);
			physBody->setUserPointer(this);
			engine->dynamicsWorld->addRigidBody(physBody);

			physRaycaster = new btDefaultVehicleRaycaster(engine->dynamicsWorld);
			btRaycastVehicle::btVehicleTuning tuning;

			float travel = info.handling.suspensionUpperLimit - info.handling.suspensionLowerLimit;
			tuning.m_frictionSlip = 1.8f;
			tuning.m_maxSuspensionTravelCm = travel * 100.f;

			physVehicle = new btRaycastVehicle(tuning, physBody, physRaycaster);
			physVehicle->setCoordinateSystem(0, 2, 1);
			physBody->setActivationState(DISABLE_DEACTIVATION);
			engine->dynamicsWorld->addVehicle(physVehicle);

			float kC = 0.4f;
			float kR = 0.6f;

			for(size_t w = 0; w < info.wheels.size(); ++w) {
				btVector3 connection(info.wheels[w].position.x, info.wheels[w].position.y, info.wheels[w].position.z - info.handling.suspensionLowerLimit);
				bool front = connection.y() > 0;
				btWheelInfo& wi = physVehicle->addWheel(connection + com, btVector3(0.f, 0.f, -1.f), btVector3(1.f, 0.f, 0.f), travel, veh->wheelScale / 2.f, tuning, front);
				wi.m_suspensionStiffness = info.handling.suspensionForce * 10.f;
				wi.m_wheelsDampingCompression = kC * 2.f * btSqrt(wi.m_suspensionStiffness);
				wi.m_wheelsDampingRelaxation = kR * 2.f * btSqrt(wi.m_suspensionStiffness);
				wi.m_rollInfluence = 0.2f;
				wi.m_frictionSlip = tuning.m_frictionSlip * (front ? info.handling.tractionBias : 1.f - info.handling.tractionBias);
			}

		}
	}
}

glm::vec3 GTAVehicle::getPosition() const
{
	if(physBody) {
		btVector3 Pos = physBody->getWorldTransform().getOrigin();
		return glm::vec3(Pos.x(), Pos.y(), Pos.z()) + info.handling.centerOfMass;
	}
	return position;
}

glm::quat GTAVehicle::getRotation() const
{
	if(physBody) {
		btQuaternion rot = physBody->getWorldTransform().getRotation();
		return glm::quat(rot.w(), rot.x(), rot.y(), rot.z());
	}
	return rotation;
}

void GTAVehicle::tick(float dt)
{
	if(physVehicle) {
		for(size_t w = 0; w < physVehicle->getNumWheels(); ++w) {
			btWheelInfo& wi = physVehicle->getWheelInfo(w);
			if( info.handling.driveType == VehicleHandlingInfo::All ||
					(info.handling.driveType == VehicleHandlingInfo::Forward && wi.m_bIsFrontWheel) ||
					(info.handling.driveType == VehicleHandlingInfo::Rear && !wi.m_bIsFrontWheel))
			{
				physVehicle->applyEngineForce(info.handling.acceleration * 150.f * throttle, w);
			}

			float brakeReal = info.handling.brakeDeceleration * info.handling.mass * (wi.m_bIsFrontWheel? info.handling.brakeBias : 1.f - info.handling.brakeBias);
			physVehicle->setBrake(brakeReal * brake, w);

			if(wi.m_bIsFrontWheel) {
				float sign = std::signbit(steerAngle) ? -1.f : 1.f;
				physVehicle->setSteeringValue(std::min(info.handling.steeringLock*(3.141f/180.f), std::abs(steerAngle)) * sign, w);
				//physVehicle->setSteeringValue(std::min(3.141f/2.f, std::abs(steerAngle)) * sign, w);
			}
		}
	}
}

void GTAVehicle::setSteeringAngle(float a)
{
	steerAngle = a;
}

float GTAVehicle::getSteeringAngle() const
{
	return steerAngle;
}

void GTAVehicle::setThrottle(float t)
{
	throttle = t;
}

float GTAVehicle::getThrottle() const
{
	return throttle;
}

void GTAVehicle::setBraking(float b)
{
	brake = b;
}

float GTAVehicle::getBraking() const
{
	return brake;
}

void GTAVehicle::setHandbraking(bool hb)
{
	handbrake = hb;
}

bool GTAVehicle::getHandbraking() const
{
	return handbrake;
}
