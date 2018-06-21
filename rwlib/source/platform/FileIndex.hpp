#ifndef _LIBRW_FILEINDEX_HPP_
#define _LIBRW_FILEINDEX_HPP_

#include "rw/filesystem.hpp"
#include "rw/forward.hpp"

#include <unordered_map>

class FileIndex {
public:
    /**
     * @brief normalizeString Normalize a file path
     * @param filePath the path to normalize
     * @return normalized file path
     */
    static std::string normalizeFilePath(const std::string &filePath);

    /**
     * @brief indexDirectory index all files at path
     * @param path the path to index
     *
     * This is used to build the mapping of lower-case file paths to the
     * true case on the file system for platforms where this is an issue.
     */
    void indexTree(const rwfs::path &path);

    /**
     * @brief findFilePath finds disk path for a game data file
     * @param filePath the path to find
     * @return The file path as it exists on disk
     * @throws if this FileIndex has not indexed the path
     */
    rwfs::path findFilePath(const std::string &filePath) const;

    /**
     * @brief openFileRaw Opens a raw file on the disk
     * @param filePath the path to open
     * @return FileHandle to the file
     * @throws if this FileIndex has not indexed the path
     */
    FileHandle openFileRaw(const std::string &filePath) const;

    /**
     * Adds the files contained within the given Archive file to the
     * file index.
     * @param filePath path to the archive
     * @throws if this FileIndex has not indexed the archive itself
     */
    void indexArchive(const std::string &filePath);

    /**
     * Returns a FileHandle for the file if it can be found in the
     * file index, otherwise an empty FileHandle is returned.
     * @param filePath name of the file to open
     * @return FileHandle to the file, nullptr if this FileINdexed has not indexed the path
     */
    FileHandle openFile(const std::string &filePath);

private:
    /**
     * @brief Type of the indexed data.
     */
    enum IndexedDataType {
        /// Is a file on disk
        FILE,
        /// Is a member of an archive
        ARCHIVE,
    };

    /**
     * @brief All information of indexed data.
     */
    struct IndexedData {
        /// Type of indexed data.
        IndexedDataType type;
        /// Path of indexed data.
        std::string path;
        /// Extra data of assets (FIXME: use c++17 std::variant or std::option)
        std::string assetData;
    };

    /**
     * @brief indexedData_ A mapping from filepath (relative to game data path) to an IndexedData item.
     */
    std::unordered_map<std::string, IndexedData> indexedData_;

    /**
     * @brief getIndexedDataAt Get IndexedData for filePath
     * @param filePath the file path to get the IndexedData for
     * @return IndexedData pointer if this FileIndex has indexed the filePath
     * @throws If this FileIndex has not indexed filePath
     */
    const IndexedData *getIndexedDataAt(const std::string &filePath) const;
};

#endif
