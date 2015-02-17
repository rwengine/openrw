#include <boost/test/unit_test.hpp>
#include "test_globals.hpp"

#include <ai/TrafficDirector.hpp>
#include <ai/AIGraph.hpp>
#include <objects/InstanceObject.hpp>
#include <objects/CharacterObject.hpp>

bool operator!=(const AIGraphNode* lhs, const glm::vec3 &rhs) { return lhs->position != rhs; }
std::ostream &operator<<(std::ostream &os, const AIGraphNode* yt) { os << glm::to_string(yt->position); return os; }

BOOST_AUTO_TEST_SUITE(TrafficDirectorTests)

BOOST_AUTO_TEST_CASE(test_available_nodes)
{
	AIGraph graph;
	
	PathData path {
		PathData::PATH_PED,
		0, "",
		{
			{
				PathNode::EXTERNAL,
				1,
				{ 10.f, 10.f, 0.f },
				1.f, 0, 0
			},
			{
				PathNode::EXTERNAL,
				2,
				{ 10.f, 0.f, 0.f },
				1.f, 0, 0
			},
			{
				PathNode::EXTERNAL,
				3,
				{ 10.f,-10.f, 0.f },
				1.f, 0, 0
			},
			{
				PathNode::EXTERNAL,
				4,
				{ 0.f,-10.f, 0.f },
				1.f, 0, 0
			},
			{
				PathNode::EXTERNAL,
				5,
				{-10.f,-10.f, 0.f },
				1.f, 0, 0
			},
			{
				PathNode::EXTERNAL,
				6,
				{-10.f, 0.f, 0.f },
				1.f, 0, 0
			},
			{
				PathNode::EXTERNAL,
				-1,
				{-10.f, 10.f, 0.f },
				1.f, 0, 0
			},
		}
	};
	
	graph.createPathNodes(glm::vec3(), glm::quat(), path);
	
	TrafficDirector director(&graph, Global::get().e);
	
	auto open = director.findAvailableNodes(AIGraphNode::Pedestrian, glm::vec3(-5.f, -5.f, 0.f), 10.f);
	
	std::vector<glm::vec3> expected {
			{ 0.f,-10.f, 0.f },
			{-10.f,-10.f, 0.f },
			{-10.f, 0.f, 0.f }
	};
	
	BOOST_ASSERT( expected.size() == open.size() );
	
	auto left = open.begin();
	auto right = expected.begin();
	for ( ; left != open.end() && right != expected.end(); ++left, ++right )
	{
		BOOST_CHECK( ! ( *left != *right ) );
	}
}

BOOST_AUTO_TEST_CASE(test_node_not_blocking)
{
	AIGraph graph;
	
	PathData path {
		PathData::PATH_PED,
		0, "",
		{
			{
				PathNode::EXTERNAL,
				1,
				{ 10.f, 10.f, 0.f },
				1.f, 0, 0
			},
		}
	};
	
	graph.createPathNodes(glm::vec3(), glm::quat(), path);
	
	TrafficDirector director(&graph, Global::get().e);
	
	// Create something that isn't a pedestrian
	InstanceObject* box = Global::get().e->createInstance(1337, glm::vec3(10.f, 10.f, 0.f));
	
	{
		auto open = director.findAvailableNodes(AIGraphNode::Pedestrian, glm::vec3( 5.f,  5.f, 0.f), 10.f);
		BOOST_CHECK( open.size() == 1 );
	}
	
	Global::get().e->destroyObject(box);
}

BOOST_AUTO_TEST_CASE(test_node_blocking)
{
	AIGraph graph;
	
	PathData path {
		PathData::PATH_PED,
		0, "",
		{
			{
				PathNode::EXTERNAL,
				1,
				{ 10.f, 10.f, 0.f },
				1.f, 0, 0
			},
		}
	};
	
	graph.createPathNodes(glm::vec3(), glm::quat(), path);
	
	TrafficDirector director(&graph, Global::get().e);
	
	// create something that should block the spawn point
	CharacterObject* ped = Global::get().e->createPedestrian(1, glm::vec3(10.f, 10.f, 0.f));
	
	{
		auto open = director.findAvailableNodes(AIGraphNode::Pedestrian, glm::vec3( 5.f,  5.f, 0.f), 10.f);
		BOOST_CHECK( open.size() == 0 );
	}
	
	Global::get().e->destroyObject(ped);
}

BOOST_AUTO_TEST_CASE(test_node_density)
{
	AIGraph graph;
	
	PathData path {
		PathData::PATH_PED,
		0, "",
		{
			{
				PathNode::EXTERNAL,
				1,
				{ 10.f, 10.f, 0.f },
				1.f, 0, 0
			},
		}
	};
	
	graph.createPathNodes(glm::vec3(), glm::quat(), path);
	
	TrafficDirector director(&graph, Global::get().e);
	
	CharacterObject* ped = Global::get().e->createPedestrian(1, glm::vec3(5.f, 5.f, 0.f));
	
	{
		director.setDensity(AIGraphNode::Pedestrian, 1.f);
		auto open = director.findAvailableNodes(AIGraphNode::Pedestrian, glm::vec3( 5.f,  5.f, 0.f), 10.f);
		BOOST_CHECK( open.size() == 0 );
	}
	
	{
		director.setDensity(AIGraphNode::Pedestrian, 2.f);
		auto open = director.findAvailableNodes(AIGraphNode::Pedestrian, glm::vec3( 5.f,  5.f, 0.f), 10.f);
		BOOST_CHECK( open.size() == 1 );
	}
	
	Global::get().e->destroyObject(ped);
}
BOOST_AUTO_TEST_SUITE_END()
