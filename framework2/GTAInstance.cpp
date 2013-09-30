#include <renderwure/objects/GTAInstance.hpp>
#include <renderwure/engine/GTAEngine.hpp>

GTAInstance::GTAInstance(
		GTAEngine* engine,
		const glm::vec3& pos,
		const glm::quat& rot,
		Model* model,
		const glm::vec3& scale,
		std::shared_ptr<LoaderIDE::OBJS_t> obj,
		std::shared_ptr<GTAInstance> lod
			)
: GTAObject(engine, pos, rot, model), scale(scale), object(obj), LODinstance(lod)
{
	btRigidBody* body = nullptr;
	auto phyit = engine->gameData.collisions.find(obj->modelName);
	if( phyit != engine->gameData.collisions.end()) {
		btCompoundShape* cmpShape = new btCompoundShape;
		btDefaultMotionState* msta = new btDefaultMotionState;
		msta->setWorldTransform(btTransform(
			btQuaternion(
				rot.x, rot.y, rot.z, -rot.w
			).inverse(),
			btVector3(
				pos.x, pos.y, pos.z
			)
		));
		btRigidBody::btRigidBodyConstructionInfo info(0.f, msta, cmpShape);
		CollisionInstance& physInst = *phyit->second.get();

		// Boxes
		for( size_t i = 0; i < physInst.boxes.size(); ++i ) {
			CollTBox& box = physInst.boxes[i];
			auto size = (box.max - box.min) / 2.f;
			auto mid = (box.min + box.max) / 2.f;
			btCollisionShape* bshape = new btBoxShape( btVector3(size.x, size.y, size.z)  );
			btTransform t; t.setIdentity();
			t.setOrigin(btVector3(mid.x, mid.y, mid.z));
			cmpShape->addChildShape(t, bshape);
		}

		// Spheres
		for( size_t i = 0; i < physInst.spheres.size(); ++i ) {
			CollTSphere& sphere = physInst.spheres[i];
			btCollisionShape* sshape = new btSphereShape(sphere.radius);
			btTransform t; t.setIdentity();
			t.setOrigin(btVector3(sphere.center.x, sphere.center.y, sphere.center.z));
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
			btTransform t; t.setIdentity();
			cmpShape->addChildShape(t, trishape);
		}

		body = new btRigidBody(info);
		engine->dynamicsWorld->addRigidBody(body);
	}

	auto pathit = engine->objectNodes.find(obj->ID);
	if( pathit != engine->objectNodes.end() ) {
		auto& pathlist = pathit->second;
		for( size_t p = 0; p < pathlist.size(); ++p ) {
			auto& path = pathlist[p];
			engine->aigraph.createPathNodes(position, rot, path);
		}
	}

}

