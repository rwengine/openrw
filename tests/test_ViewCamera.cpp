#include <boost/test/unit_test.hpp>
#include "test_Globals.hpp"
#include <render/ViewCamera.hpp>

namespace {

struct CameraFixture {
    ViewCamera camera_ {
        {1.f, 2.f, 3.f}
    };
};
}

BOOST_AUTO_TEST_SUITE(ViewCameraTests)

BOOST_FIXTURE_TEST_CASE(test_creation, CameraFixture) {
    BOOST_CHECK_EQUAL(camera_.position, glm::vec3(1.f, 2.f, 3.f));
}

BOOST_FIXTURE_TEST_CASE(test_view_matrix, CameraFixture) {
    const auto& view = camera_.getView();
    BOOST_CHECK_EQUAL(view[3], glm::vec4(2.f, -3.f, 1.f, 1.f));
}


BOOST_AUTO_TEST_SUITE_END()
