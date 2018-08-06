#ifndef _LIBRW_FILEHANDLE_HPP_
#define _LIBRW_FILEHANDLE_HPP_

#include <cstddef>
#include <memory>

/**
 * @brief Contains a pointer to a file's contents.
 */
struct FileContentsInfo {
    std::unique_ptr<char[]> data;
    size_t length;

    FileContentsInfo(std::unique_ptr<char[]> mem, size_t len)
        : data(std::move(mem)), length(len) {
    }

    FileContentsInfo(FileContentsInfo&& info)
        : data(std::move(info.data)), length(info.length) {
        info.data = nullptr;
    }

    FileContentsInfo(FileContentsInfo& info) = delete;
    FileContentsInfo& operator=(FileContentsInfo& info) = delete;

    ~FileContentsInfo() = default;
};

#endif
