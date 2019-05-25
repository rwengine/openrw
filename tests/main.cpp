#define BOOST_TEST_MODULE openrw
#include <boost/test/unit_test.hpp>
#include "test_Globals.hpp"

std::ostream& operator<<(std::ostream& stream, const glm::vec3& v) {
    stream << glm::to_string(v);
    return stream;
}
