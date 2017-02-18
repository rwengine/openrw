#define BOOST_TEST_MODULE openrw
#include <boost/test/included/unit_test.hpp>
#include "test_globals.hpp"

std::ostream& operator<<(std::ostream& stream, const glm::vec3& v) {
    stream << v.x << " " << v.y << " " << v.z;
    return stream;
}
