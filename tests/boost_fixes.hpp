#ifndef _RWTESTS_BOOST_FIXES_HPP_
#define _RWTESTS_BOOST_FIXES_HPP_

#include <fonts/GameTexts.hpp>

#include <boost/test/unit_test.hpp>
#include <glm/gtx/string_cast.hpp>

// Boost moved the print_log_value struct in version 1.59
// TODO: use another testing library
#if BOOST_VERSION >= 105900
#define BOOST_NS_MAGIC namespace tt_detail {
#define BOOST_NS_MAGIC_CLOSING }
#else
#define BOOST_NS_MAGIC
#define BOOST_NS_MAGIC_CLOSING
#endif

namespace boost {
namespace test_tools {
BOOST_NS_MAGIC
template <>
struct print_log_value<glm::vec3> {
    void operator()(std::ostream& s, glm::vec3 const& v) {
        s << glm::to_string(v);
    }
};
BOOST_NS_MAGIC_CLOSING
}
}

#if BOOST_VERSION < 106400
namespace boost {
namespace test_tools {
BOOST_NS_MAGIC
template <>
struct print_log_value<std::nullptr_t> {
    void operator()(std::ostream& s, std::nullptr_t) {
        s << "nullptr";
    }
};
BOOST_NS_MAGIC_CLOSING
}
}
#endif

namespace boost {
namespace test_tools {
BOOST_NS_MAGIC
template <>
struct print_log_value<GameString> {
    void operator()(std::ostream& s, GameString const& v) {
        for (GameString::size_type i = 0u; i < v.size(); ++i) {
            s << static_cast<char>(v[i]);
        }
    }
};
BOOST_NS_MAGIC_CLOSING
}
}

#undef BOOST_NS_MAGIC
#undef BOOST_NS_MAGIC_CLOSING
#endif
