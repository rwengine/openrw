#ifndef RWENGINE_FILEHANDLE_HPP
#define RWENGINE_FILEHANDLE_HPP

#include <memory>

/**
 * @brief Contains a pointer to a file's contents.
 */
struct FileContentsInfo
{
	char* data;
	size_t length;

	~FileContentsInfo() {
		delete[] data;
	}
};

typedef std::shared_ptr<FileContentsInfo> FileHandle;

#endif
