#pragma once
#include "FileHandle.hpp"

#include <string>
#include <map>

class FileIndex
{
public:

	struct IndexData
	{
		/// Lowercase identifying filename
		std::string filename;
		/// Original filename
		std::string originalName;
		/// Containing directory
		std::string directory;
		/// The archive filename (if applicable)
		std::string archive;
	};

	/**
	 * Adds the files contained within the given directory to the
	 * file index.
	 */
	void indexDirectory(const std::string& directory);
	
	/**
	 * Adds the files contained within the given directory tree to the
	 * file index.
	 */
	void indexTree(const std::string& root);
	
	/**
	 * Adds the files contained within the given Archive file to the
	 * file index.
	 */
	void indexArchive(const std::string& archive);
	
	/**
	 * Returns true if the file identified by filename is found within
	 * the file index. If the file is found, the filedata parameter
	 * is populated.
	 */
	bool findFile(const std::string& filename, IndexData& filedata);

	/**
	 * Returns a FileHandle for the file if it can be found in the
	 * file index, otherwise an empty FileHandle is returned.
	 */
	FileHandle openFile(const std::string& filename);

private:
	std::map<std::string, IndexData> files;
};