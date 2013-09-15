#include <renderwure/objects/GTAVehicle.hpp>
#include <renderwure/engine/GTAEngine.hpp>
#include <BulletDynamics/Vehicle/btRaycastVehicle.h>

GTAVehicle::GTAVehicle(GTAEngine* engine, const glm::vec3& pos, const glm::quat& rot, Model* model, std::shared_ptr<LoaderIDE::CARS_t> veh, const VehicleInfo& info, const glm::vec3& prim, const glm::vec3& sec)
	: GTAObject(engine, pos, rot, model), vehicle(veh), info(info), colourPrimary(prim), colourSecondary(sec), physBody(nullptr), physVehicle(nullptr)
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
			CollisionInstance& physInst = *phyit->second.get();

			btVector3 com(info.handling.centerOfMass.x, info.handling.centerOfMass.y, info.handling.centerOfMass.z);

			// Boxes
			for( size_t i = 0; i < physInst.boxes.size(); ++i ) {
				CollTBox& box = physInst.boxes[i];
				auto size = (box.max - box.min) / 2.f;
				auto mid = (box.min + box.max) / 2.f;
				btCollisionShape* bshape = new btBoxShape( btVector3(size.x, size.y, size.z)  );
				btTransform t(btQuaternion(0.f, 0.f, 0.f, 1.f), btVector3(mid.x, mid.y, mid.z));
				cmpShape->addChildShape(t, bshape);
			}

			// Spheres
			for( size_t i = 0; i < physInst.spheres.size(); ++i ) {
				CollTSphere& sphere = physInst.spheres[i];
				btCollisionShape* sshape = new btSphereShape(sphere.radius);
				btTransform t(btQuaternion(0.f, 0.f, 0.f, 1.f), btVector3(sphere.center.x, sphere.center.y, sphere.center.z));
				cmpShape->addChildShape(t, sshape);
			}

			if( physInst.triangles.size() > 0 ) {
				btTriangleIndexVertexArray* vertarray = new btTriangleIndexVertexArray(
							physInst.triangles.size(),
							(int*) physInst.triangles.data(),
							sizeof(CollTFaceTriangle),
							physInst.vertices.size(),
							&(physInst.vertices[0].x),
						sizeof(glm::vec3)
						);
				btBvhTriangleMeshShape* trishape = new btBvhTriangleMeshShape(vertarray, false);
				cmpShape->addChildShape(
							btTransform(btQuaternion(0.f, 0.f, 0.f, 1.f), btVector3(0.f, 0.f, 0.f)),
							trishape
							);
			}

			btVector3 inertia(0,0,0);
			cmpShape->calculateLocalInertia(info.handling.mass, inertia);

			btRigidBody::btRigidBodyConstructionInfo rginfo(info.handling.mass, msta, cmpShape, inertia);

			physBody = new btRigidBody(rginfo);
			engine->dynamicsWorld->addRigidBody(physBody);

			physRaycaster = new btDefaultVehicleRaycaster(engine->dynamicsWorld);
			btRaycastVehicle::btVehicleTuning tuning;

			float travel = info.handling.suspensionUpperLimit - info.handling.suspensionLowerLimit;
			tuning.m_maxSuspensionTravelCm = (travel)*100.f;
			tuning.m_frictionSlip = info.handling.tractionMulti * 10.f;

			physVehicle = new btRaycastVehicle(tuning, physBody, physRaycaster);
			physVehicle->setCoordinateSystem(0, 2, 1);
			physBody->setActivationState(DISABLE_DEACTIVATION);
			engine->dynamicsWorld->addVehicle(physVehicle);

			for(size_t w = 0; w < info.wheels.size(); ++w) {
				btVector3 connection(info.wheels[w].position.x, info.wheels[w].position.y, info.wheels[w].position.z);
				bool front = connection.y() > 0;
				btWheelInfo& wi = physVehicle->addWheel(connection, btVector3(0.f, 0.f, -1.f), btVector3(1.f, 0.f, 0.f), travel*0.45f, veh->wheelScale / 2.f, tuning, front);
				wi.m_wheelsSuspensionForce = info.handling.mass * 1.5f;
				wi.m_suspensionStiffness = 20.f;
				wi.m_wheelsDampingRelaxation = 2.3;
				wi.m_wheelsDampingCompression = 4.4f;
				wi.m_frictionSlip = tuning.m_frictionSlip * (front ? info.handling.tractionBias : 1.f - info.handling.tractionBias);
			}

		}
	}
}

glm::vec3 GTAVehicle::getPosition() const
{
	if(physBody) {
		btVector3 Pos = physBody->getWorldTransform().getOrigin();
		return glm::vec3(Pos.x(), Pos.y(), Pos.z());
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
				physVehicle->applyEngineForce(info.handling.acceleration * info.handling.mass * 0.1f, w);
			}

			/*if(wi.m_bIsFrontWheel) {
				physVehicle->setSteeringValue(info.handling.steeringLock*(3.141/180.f), w);
			}*/
		}
	}
}
