#include "ai/TrafficDirector.hpp"
#include <ai/AIGraphNode.hpp>
#include <ai/CharacterController.hpp>
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
		if ( ! node->external ) continue;
		if ( node->type != type ) continue;
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
	
	/// Hardcoded cop Pedestrian
	std::vector<uint16_t> validPeds = { 1 };
	validPeds.insert(validPeds.end(), {20, 11, 19, 5});
	std::random_device rd;
	std::default_random_engine re(rd());
	std::uniform_int_distribution<> d(0, validPeds.size()-1);
	
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
		
		// Spawn a pedestrian from the available pool
		auto ped = world->createPedestrian(validPeds[d(re)], spawn->position + glm::vec3( 0.f, 0.f, 1.f ) );
		ped->setLifetime(GameObject::TrafficLifetime);
		ped->controller->setGoal(CharacterController::TrafficWander);
		created.push_back( ped );
	}
	
	// Find places it's legal to spawn things
	
	return created;
}

