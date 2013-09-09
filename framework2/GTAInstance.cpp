#include <renderwure/objects/GTAInstance.hpp>
#include <renderwure/engine/GTAEngine.hpp>

GTAInstance::GTAInstance(
		GTAEngine* engine,
		const glm::vec3& pos,
		const glm::quat& rot,
		Model* model,
		const glm::vec3& scale,
		LoaderIPLInstance inst,
		std::shared_ptr<LoaderIDE::OBJS_t> obj,
		std::shared_ptr<GTAInstance> lod
			)
: GTAObject(engine, pos, rot, model), scale(scale), instance(inst), object(obj), LODinstance(lod)
{
	btRigidBody* body = nullptr;
	auto phyit = engine->gameData.collisions.find(obj->modelName);
	if( phyit != engine->gameData.collisions.end()) {
		btCompoundShape* cmpShape = new btCompoundShape;
		btDefaultMotionState* msta = new btDefaultMotionState;
		msta->setWorldTransform(btTransform(
			btQuaternion(
				inst.rotX, inst.rotY, inst.rotZ, inst.rotW
			).inverse(),
			btVector3(
				inst.posX, inst.posY, inst.posZ
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

		body = new btRigidBody(info);
		engine->dynamicsWorld->addRigidBody(body);
	}

	auto pathit = engine->objectNodes.find(inst.id);
	if( pathit != engine->objectNodes.end() ) {
		auto& pathlist = pathit->second;
		for( size_t p = 0; p < pathlist.size(); ++p ) {
			auto& path = pathlist[p];
			size_t startIndex = engine->ainodes.size();
			for( size_t n = 0; n < path.nodes.size(); ++n ) {
				auto& node = path.nodes[n];
				GTAAINode* ainode = new GTAAINode;

				ainode->type = (path.type == LoaderIDE::PATH_PED ? GTAAINode::Pedestrian : GTAAINode::Vehicle);
				ainode->nextIndex = node.next >= 0 ? startIndex + node.next : -1;
				ainode->flags = GTAAINode::None;
				ainode->position = position + (rotation * node.position);

				if( node.type == LoaderIDE::EXTERNAL ) {
					ainode->flags |= GTAAINode::External;
					for( size_t rn = 0; rn < engine->ainodes.size(); ++rn ) {
						if( (engine->ainodes[rn]->flags & GTAAINode::External) == GTAAINode::External ) {
							auto d = glm::length(engine->ainodes[rn]->position - ainode->position);
							if( d < 1.f ) {
								ainode->connections.push_back(engine->ainodes[rn]);
								engine->ainodes[rn]->connections.push_back(ainode);
							}
						}
					}
				}

				if( ainode->nextIndex < engine->ainodes.size() ) {
					ainode->connections.push_back(engine->ainodes[ainode->nextIndex]);
					engine->ainodes[ainode->nextIndex]->connections.push_back(ainode);
				}

				for( size_t on = startIndex; on < engine->ainodes.size(); ++on ) {
					if( engine->ainodes[on]->nextIndex == startIndex+n ) {
						engine->ainodes[on]->connections.push_back(ainode);
						ainode->connections.push_back(engine->ainodes[on]);
					}
				}

				engine->ainodes.push_back(ainode);

			}
		}
	}

}

