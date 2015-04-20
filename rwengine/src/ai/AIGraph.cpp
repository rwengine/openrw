#include "ai/AIGraph.hpp"
#include "engine/GameObject.hpp"
#include <ai/AIGraphNode.hpp>
#include <glm/gtx/norm.hpp>
#include <iostream>

AIGraph::~AIGraph()
{
	for( auto n : nodes ) {
		delete n;
	}
}

void AIGraph::createPathNodes(const glm::vec3& position, const glm::quat& rotation, PathData& path)
{
	size_t startIndex = nodes.size();
	std::vector<AIGraphNode*> pathNodes;
	pathNodes.reserve(path.nodes.size());

	for( size_t n = 0; n < path.nodes.size(); ++n ) {
		auto& node = path.nodes[n];
		AIGraphNode* ainode = nullptr;
		glm::vec3 nodePosition = position + (rotation * node.position);

		if( node.type == PathNode::EXTERNAL ) {
			for( size_t rn = 0; rn < externalNodes.size(); ++rn ) {
				auto& realNode = externalNodes[rn];
				auto d = glm::distance2(realNode->position, nodePosition);
				if( d < 1.f ) {
					pathNodes.push_back(realNode);
					ainode = realNode;
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

			pathNodes.push_back(ainode);
			nodes.push_back(ainode);

			if( ainode->external )
			{
				externalNodes.push_back(ainode);
				
				// Determine which grid cell this node falls into
				float lowerCoord = -(WORLD_GRID_SIZE)/2.f;
				auto gridrel = glm::vec2(ainode->position) - glm::vec2(lowerCoord, lowerCoord);
				auto gridcoord = glm::floor(gridrel / glm::vec2(WORLD_CELL_SIZE));
				if( gridcoord.x < 0 || gridcoord.y < 0 || gridcoord.x >= WORLD_GRID_WIDTH || gridcoord.y >= WORLD_GRID_WIDTH )
				{
					std::cout << "Warning: Node outside of grid at coord " << gridcoord.x << " " << gridcoord.y << std::endl;
				}
				auto index = (gridcoord.x * WORLD_GRID_WIDTH) + gridcoord.y;
				gridNodes[index].push_back(ainode);
			}
		}
	}

	for(size_t pn = 0; pn < path.nodes.size(); ++pn) {
		if(path.nodes[pn].next >= 0 && (unsigned) path.nodes[pn].next < pathNodes.size()) {
			auto node = pathNodes[pn];
			auto next = pathNodes[path.nodes[pn].next];
			
			node->connections.push_back(next);
			next->connections.push_back(node);
		}
	}
}

glm::ivec2 worldToGrid(const glm::vec2& world)
{
	static const float lowerCoord = -(WORLD_GRID_SIZE)/2.f;
	return glm::ivec2((world - glm::vec2(lowerCoord)) / glm::vec2(WORLD_CELL_SIZE));
}

void AIGraph::gatherExternalNodesNear(const glm::vec3& center, const float radius, std::vector< AIGraphNode* >& nodes)
{
	// the bounds end up covering more than might fit
	auto planecoords = glm::vec2(center);
	auto minWorld = planecoords - glm::vec2(radius);
	auto maxWorld = planecoords + glm::vec2(radius);
	auto minGrid = worldToGrid(minWorld);
	auto maxGrid = worldToGrid(maxWorld);

	for( int x = minGrid.x; x <= maxGrid.x; ++x )
	{
		for( int y = minGrid.y; y <= maxGrid.y; ++y )
		{
			int i = (x * WORLD_GRID_WIDTH) + y;
			if( i < 0 || i >= gridNodes.size() )
			{
				continue;
			}
			auto& external = gridNodes[i];
			for ( AIGraphNode* node : external )
			{
				if ( glm::distance2( center, node->position ) < radius*radius )
				{
					nodes.push_back( node );
				}
			}
		}
	}
}

