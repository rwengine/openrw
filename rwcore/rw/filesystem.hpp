#ifndef _LIBRW_FILESYSTEM_HPP_
#define _LIBRW_FILESYSTEM_HPP_

#define RW_FS_CXX17 0
#define RW_FS_CXXTS 1
#define RW_FS_BOOST 2

#if RW_FS_LIBRARY == RW_FS_CXX17
#include <filesystem>
#include <system_error>
namespace rwfs {
    using namespace std::filesystem;
    using error_code = std::error_code;
}
#elif RW_FS_LIBRARY == RW_FS_CXXTS
#include <experimental/filesystem>
#include <system_error>
namespace rwfs {
    using namespace std::experimental::filesystem;
    using error_code = std::error_code;
}
#elif RW_FS_LIBRARY == RW_FS_BOOST
#include <boost/filesystem.hpp>
#include <boost/functional/hash.hpp>
#include <boost/system/error_code.hpp>
namespace rwfs {
    using namespace boost::filesystem;
    using error_code = boost::system::error_code;
}
#include <boost/version.hpp>
#if BOOST_VERSION < 105600
namespace boost {
namespace filesystem {
inline const directory_iterator& begin(const directory_iterator& iter) {
    return iter;
}
inline directory_iterator end(const directory_iterator&) {
    return {};
}
inline const recursive_directory_iterator& begin(const recursive_directory_iterator& iter) {
    return iter;
}
inline recursive_directory_iterator end(const recursive_directory_iterator&) {
    return {};
}
}
}
#endif
#else
#error Invalid RW_FS_LIBRARY value
#endif

namespace std {
template <>
struct hash<rwfs::path> {
    size_t operator()(const rwfs::path& p) const {
        return rwfs::hash_value(p);
    }
};
}

#endif
