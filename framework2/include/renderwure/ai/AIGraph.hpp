#pragma once
#ifndef _AIGRAPH_HPP_
#define _AIGRAPH_HPP_
#include <vector>
#include <glm/gtc/quaternion.hpp>
#include <renderwure/loaders/LoaderIDE.hpp>

struct GTAInstance;
struct GTAAINode;

class AIGraph
{
public:
	
	std::vector<GTAAINode*> nodes;
	
	void createPathNodes(const glm::vec3& position, const glm::quat& rotation, LoaderIDE::PATH_t& path);

};

#endif