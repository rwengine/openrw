#include <renderwure/objects/GTAVehicle.hpp>
#include <renderwure/engine/GTAEngine.hpp>

GTAVehicle::GTAVehicle(GTAEngine* engine, const glm::vec3& pos, const glm::quat& rot, Model* model, std::shared_ptr<LoaderIDE::CARS_t> veh, const glm::vec3& prim, const glm::vec3& sec)
	: GTAObject(engine, pos, rot, model), vehicle(veh), colourPrimary(prim), colourSecondary(sec), physBody(nullptr)
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
			cmpShape->calculateLocalInertia(1000.f, inertia);

			btRigidBody::btRigidBodyConstructionInfo info(1000.f, msta, cmpShape, inertia);

			physBody = new btRigidBody(info);
			engine->dynamicsWorld->addRigidBody(physBody);
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
