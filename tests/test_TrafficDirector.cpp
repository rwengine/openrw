#include <boost/test/unit_test.hpp>
#include "test_Globals.hpp"

#include <ai/AIGraph.hpp>
#include <ai/AIGraphNode.hpp>
#include <ai/TrafficDirector.hpp>
#include <data/PathData.hpp>
#include <objects/CharacterObject.hpp>
#include <objects/InstanceObject.hpp>
#include <render/ViewCamera.hpp>

bool operator!=(const ai::AIGraphNode* lhs, const glm::vec3& rhs) {
    return lhs->position != rhs;
}
std::ostream& operator<<(std::ostream& os, const ai::AIGraphNode* yt) {
    os << glm::to_string(yt->position);
    return os;
}

BOOST_AUTO_TEST_SUITE(TrafficDirectorTests)

#if RW_TEST_WITH_DATA
BOOST_AUTO_TEST_CASE(test_available_nodes) {
    ai::AIGraph graph;

    PathData path{PathData::PATH_PED,
                  0,
                  "",
                  {
                      {PathNode::EXTERNAL, 1, {10.f, 10.f, 0.f}, 1.f, 0, 0},
                      {PathNode::EXTERNAL, 2, {10.f, 0.f, 0.f}, 1.f, 0, 0},
                      {PathNode::EXTERNAL, 3, {10.f, -10.f, 0.f}, 1.f, 0, 0},
                      {PathNode::EXTERNAL, 4, {0.f, -10.f, 0.f}, 1.f, 0, 0},
                      {PathNode::EXTERNAL, 5, {-10.f, -10.f, 0.f}, 1.f, 0, 0},
                      {PathNode::EXTERNAL, 6, {-10.f, 0.f, 0.f}, 1.f, 0, 0},
                      {PathNode::EXTERNAL, -1, {-10.f, 10.f, 0.f}, 1.f, 0, 0},
                  }};

    graph.createPathNodes(glm::vec3(), glm::quat{1.0f,0.0f,0.0f,0.0f}, path);

    ai::TrafficDirector director(&graph, Global::get().e);

    ViewCamera testCamera(glm::vec3(-5.f, -5.f, 0.f));

    auto open =
        director.findAvailableNodes(ai::NodeType::Pedestrian, testCamera, 10.f);

    std::vector<glm::vec3> expected{
        {0.f, -10.f, 0.f}, {-10.f, -10.f, 0.f}, {-10.f, 0.f, 0.f}};

    BOOST_REQUIRE(expected.size() == 3);
    BOOST_ASSERT(expected.size() == open.size());

    for (auto& v : expected) {
        BOOST_CHECK(std::find_if(open.begin(), open.end(), [v](ai::AIGraphNode* n) {
                        return n->position == v;
                    }) != open.end());
    }
}

BOOST_AUTO_TEST_CASE(test_node_not_blocking) {
    ai::AIGraph graph;

    PathData path{PathData::PATH_PED,
                  0,
                  "",
                  {
                      {PathNode::EXTERNAL, 1, {10.f, 10.f, 0.f}, 1.f, 0, 0},
                  }};

    graph.createPathNodes(glm::vec3(), glm::quat{1.0f,0.0f,0.0f,0.0f}, path);

    ai::TrafficDirector director(&graph, Global::get().e);

    // Create something that isn't a pedestrian
    InstanceObject* box =
        Global::get().e->createInstance(1337, glm::vec3(10.f, 10.f, 0.f));

    {
        auto open = director.findAvailableNodes(ai::NodeType::Pedestrian,
                                                glm::vec3(5.f, 5.f, 0.f), 10.f);
        BOOST_CHECK(open.size() == 1);
    }

    Global::get().e->destroyObject(box);
}

BOOST_AUTO_TEST_CASE(test_node_blocking) {
    ai::AIGraph graph;

    PathData path{PathData::PATH_PED,
                  0,
                  "",
                  {
                      {PathNode::EXTERNAL, 1, {10.f, 10.f, 0.f}, 1.f, 0, 0},
                  }};

    graph.createPathNodes(glm::vec3(), glm::quat{1.0f,0.0f,0.0f,0.0f}, path);

    ai::TrafficDirector director(&graph, Global::get().e);

    // create something that should block the spawn point
    CharacterObject* ped =
        Global::get().e->createPedestrian(1, glm::vec3(10.f, 10.f, 0.f));

    {
        auto open = director.findAvailableNodes(ai::NodeType::Pedestrian,
                                                glm::vec3(5.f, 5.f, 0.f), 10.f);
        BOOST_CHECK(open.size() == 0);
    }

    Global::get().e->destroyObject(ped);
}

BOOST_AUTO_TEST_CASE(test_node_density) {
    ai::AIGraph graph;

    PathData path{PathData::PATH_PED,
                  0,
                  "",
                  {
                      {PathNode::EXTERNAL, 1, {10.f, 10.f, 0.f}, 1.f, 0, 0},
                  }};

    graph.createPathNodes(glm::vec3(), glm::quat{1.0f,0.0f,0.0f,0.0f}, path);

    ai::TrafficDirector director(&graph, Global::get().e);

    CharacterObject* ped =
        Global::get().e->createPedestrian(1, glm::vec3(5.f, 5.f, 0.f));

    {
        director.setDensity(ai::NodeType::Pedestrian, 1.f);
        auto open = director.findAvailableNodes(ai::NodeType::Pedestrian,
                                                glm::vec3(5.f, 5.f, 0.f), 10.f);
        BOOST_CHECK(open.size() == 0);
    }

    {
        director.setDensity(ai::NodeType::Pedestrian, 2.f);
        auto open = director.findAvailableNodes(ai::NodeType::Pedestrian,
                                                glm::vec3(5.f, 5.f, 0.f), 10.f);
        BOOST_CHECK(open.size() == 1);
    }

    Global::get().e->destroyObject(ped);
}

BOOST_AUTO_TEST_CASE(test_create_traffic) {
    ai::AIGraph graph;

    PathData path{PathData::PATH_PED,
                  0,
                  "",
                  {
                      {PathNode::EXTERNAL, 1, {10.f, 10.f, 0.f}, 1.f, 0, 0},
                  }};

    graph.createPathNodes(glm::vec3(), glm::quat{1.0f,0.0f,0.0f,0.0f}, path);

    ai::TrafficDirector director(&graph, Global::get().e);

    auto created = director.populateNearby(glm::vec3(0.f, 0.f, 0.f), 20.f);

    BOOST_CHECK(created.size() == 1);

    // Global::get().e->destroyObject(created[0]);
}
#endif

BOOST_AUTO_TEST_SUITE_END()
