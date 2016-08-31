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

	FileContentsInfo(char* mem, size_t len)
		: data(mem), length(len)
	{

	}

	~FileContentsInfo() {
		delete[] data;
	}
};

using FileHandle = std::shared_ptr<FileContentsInfo>;

#endif
