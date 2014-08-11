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
	
	void createPathNodes(const glm::vec3& position, const glm::quat& rotation, PathData& path);

};

#endif
