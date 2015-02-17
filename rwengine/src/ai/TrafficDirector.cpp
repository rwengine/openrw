#include "ai/TrafficDirector.hpp"
#include <ai/AIGraphNode.hpp>
#include <engine/GameWorld.hpp>
#include <engine/GameObject.hpp>
#include <glm/gtx/string_cast.hpp>

TrafficDirector::TrafficDirector(AIGraph* graph, GameWorld* world)
: graph( graph ), world( world ), pedDensity(1.f), carDensity(1.f)
{

}

std::vector< AIGraphNode* > TrafficDirector::findAvailableNodes(AIGraphNode::NodeType type, const glm::vec3& near, float radius)
{
	std::vector<AIGraphNode*> available;
	
	for ( AIGraphNode* node : graph->nodes )
	{
		if ( node->type != type ) continue;
		if ( ! node->external ) continue;
		if ( glm::distance( near, node->position ) < radius )
		{
			available.push_back( node );
		}
	}
	
	float density = type == AIGraphNode::Vehicle ? carDensity : pedDensity;
	// Determine if anything in the open set is blocked
	for ( auto it = available.begin(); it != available.end(); )
	{
		float minDist = 10.f / density;
		bool blocked = false;
		for ( auto obj : world->objects )
		{
			if ( obj->type() != GameObject::Character ) continue;
			if ( glm::distance( (*it)->position, obj->getPosition() ) <= minDist )
			{
				blocked = true;
				break;
			}
		}
		
		if ( blocked )
		{
			it = available.erase( it );
		}
		else
		{
			it++;
		}
	}
	
	return available;
}

void TrafficDirector::setDensity(AIGraphNode::NodeType type, float density)
{
	switch ( type )
	{
		case AIGraphNode::Vehicle:
			carDensity = density;
			break;
		case AIGraphNode::Pedestrian:
			pedDensity = density;
			break;
	}
}
