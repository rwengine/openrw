#include "ai/TrafficDirector.hpp"
#include <ai/AIGraphNode.hpp>
#include <engine/GameWorld.hpp>
#include <engine/GameObject.hpp>
#include <objects/CharacterObject.hpp>

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

std::vector<GameObject*> TrafficDirector::populateNearby(const glm::vec3& center, float radius, int maxPopulation)
{
	std::vector<GameObject*> created;
	
	auto type = AIGraphNode::Pedestrian;
	auto available = findAvailableNodes(type, center, radius);
	
	for ( AIGraphNode* spawn : available )
	{
		if ( maxPopulation > -1 )
		{
			if ( maxPopulation == 0 )
			{
				break;
			}
			maxPopulation --;
		}
		
		// spawn a cop
		auto cop = world->createPedestrian(1, spawn->position + glm::vec3( 0.f, 0.f, 1.f ) );
		created.push_back( cop );
	}
	
	// Find places it's legal to spawn things
	
	return created;
}

