#ifndef RWENGINE_FILEINDEX_HPP
#define RWENGINE_FILEINDEX_HPP
#include "FileHandle.hpp"

#include <boost/filesystem.hpp>
#include <boost/functional/hash.hpp>
#include <string>
#include <map>
#include <unordered_map>

namespace fs = boost::filesystem;

namespace std
{
	template<> struct hash<fs::path>
	{
		size_t operator()(const fs::path& p) const
		{
			return fs::hash_value(p);
		}
	};
}

class FileIndex
{
private:
	/**
	 * Mapping type (lower case name) => (on disk name)
	 */
	using FileSystemMap = std::unordered_map<fs::path, fs::path>;

	fs::path gamedatapath_;
	FileSystemMap filesystemfiles_;

public:

	/**
	 * @brief indexDirectory finds the true case for each file in the tree
	 * @param base_path
	 *
	 * This is used to build the mapping of lower-case file paths to the
	 * true case on the file system for platforms where this is an issue.
	 *
	 */
	void indexGameDirectory(const fs::path& base_path);

	/**
	 * @brief findFilePath finds disk path for a game data file
	 * @param path
	 * @return The file path as it exists on disk
	 */
	fs::path findFilePath(std::string path)
	{
		auto backslash = std::string::npos;
		while ((backslash = path.find("\\")) != std::string::npos) {
			path.replace(backslash, 1, "/");
		}
		auto realpath = gamedatapath_ / path;
		std::string name = realpath.native();
		std::transform(name.begin(), name.end(), name.begin(), ::tolower);

		return filesystemfiles_[name];
	}

	/**
	 * @brief openFilePath opens a file on the disk
	 * @param file_path
	 * @return A handle for the file on the disk
	 */
	FileHandle openFilePath(const std::string& file_path);

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

#endif
