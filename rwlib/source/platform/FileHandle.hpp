#pragma once

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
