#include "ai/AIGraph.hpp"
#include "objects/InstanceObject.hpp"
#include <ai/AIGraphNode.hpp>

AIGraph::~AIGraph()
{
	for( auto n : nodes ) {
		delete n;
	}
}

void AIGraph::createPathNodes(const glm::vec3& position, const glm::quat& rotation, PathData& path)
{
	size_t startIndex = nodes.size();
	std::vector<size_t> realNodes;
	realNodes.reserve(path.nodes.size());

	for( size_t n = 0; n < path.nodes.size(); ++n ) {
		auto& node = path.nodes[n];
		AIGraphNode* ainode = nullptr;
		glm::vec3 nodePosition = position + (rotation * node.position);
		
		if( node.type == PathNode::EXTERNAL ) {
			for( size_t rn = 0; rn < nodes.size(); ++rn ) {
				auto d = glm::length(nodes[rn]->position - nodePosition);
				if( d < 1.f ) {
					realNodes.push_back(rn);
					ainode = nodes[rn];
					break;
				}
			}
		}
		
		if( ainode == nullptr ) {
			ainode = new AIGraphNode;
			ainode->type = (path.type == PathData::PATH_PED ? AIGraphNode::Pedestrian : AIGraphNode::Vehicle);
			ainode->nextIndex = node.next >= 0 ? startIndex + node.next : -1;
			ainode->flags = AIGraphNode::None;
			ainode->size = node.size;
			ainode->other_thing = node.other_thing;
			ainode->other_thing2 = node.other_thing2;
			ainode->position = nodePosition;
			ainode->external = node.type == PathNode::EXTERNAL;
			ainode->disabled = false;
			
			realNodes.push_back(nodes.size());
			nodes.push_back(ainode);
		}
	}

	for(size_t pn = 0; pn < path.nodes.size(); ++pn) {
		if(path.nodes[pn].next >= 0 && (unsigned) path.nodes[pn].next < realNodes.size()) {
			auto node = nodes[realNodes[pn]];
			node->connections.push_back(nodes[realNodes[path.nodes[pn].next]]);
			nodes[realNodes[path.nodes[pn].next]]->connections.push_back(node);
		}
	}
}
