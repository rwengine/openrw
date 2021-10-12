#ifndef _LIBRW_FILESYSTEM_HPP_
#define _LIBRW_FILESYSTEM_HPP_

#define RW_FS_CXX17 0
#define RW_FS_CXXTS 1

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
