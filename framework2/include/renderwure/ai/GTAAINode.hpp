#pragma once
#ifndef _GTAAINODE_HPP_
#define _GTAAINODE_HPP_
#include <glm/glm.hpp>
#include <cstdint>
#include <vector>

struct GTAAINode
{
    enum NodeType {
        Vehicle,
        Pedestrian
    };

    enum {
        None = 0,
		CrossesRoad = 1 /// No documentation for other flags yet, but this is mentioned.
    };

    NodeType type;
    glm::vec3 position;
	bool external;
	uint8_t flags;

    int32_t nextIndex;
	
	std::vector<GTAAINode*> connections;
};

#endif
