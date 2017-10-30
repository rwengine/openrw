#ifndef _LIBRW_FILEHANDLE_HPP_
#define _LIBRW_FILEHANDLE_HPP_

#include <memory>

/**
 * @brief Contains a pointer to a file's contents.
 */
struct FileContentsInfo {
    char* data;
    size_t length;

    FileContentsInfo(char* mem, size_t len) : data(mem), length(len) {
    }

    ~FileContentsInfo() {
        delete[] data;
    }
};

#endif
