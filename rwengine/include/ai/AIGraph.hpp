#pragma once
#ifndef _AIGRAPH_HPP_
#define _AIGRAPH_HPP_
#include <vector>
#include <glm/gtc/quaternion.hpp>
#include <data/PathData.hpp>

struct AIGraphNode;

class AIGraph
{
public:

	~AIGraph();
	
	std::vector<AIGraphNode*> nodes;
	/**
	 * List of external nodes, which are links between each
	 * Instance's paths and where new pedestrians and vehicles
	 * are spawned
	 */
	std::vector<AIGraphNode*> externalNodes;
	
	void createPathNodes(const glm::vec3& position, const glm::quat& rotation, PathData& path);

};

#endif
