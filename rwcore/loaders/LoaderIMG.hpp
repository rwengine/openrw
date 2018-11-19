#ifndef _LIBRW_LOADERIMG_HPP_
#define _LIBRW_LOADERIMG_HPP_

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <rw/filesystem.hpp>

/// \brief Points to one file within the archive
class LoaderIMGFile {
public:
    uint32_t offset;
    uint32_t size;
    char name[24];
};

/**
    \class LoaderIMG
    \brief Parses the structure of GTA .IMG archives and loads the files in it
*/
class LoaderIMG {
public:
    /// Multiple versions of .IMG files
    enum Version {
        GTAIIIVC,  ///< GTA III and GTA VC archives -- only this one is
                   ///implemented
        GTASA,
        GTAIV
    };

    /// Construct
    LoaderIMG() = default;

    /// Load the structure of the archive
    /// Omit the extension in filename so both .dir and .img are loaded when
    /// appropriate
    bool load(const rwfs::path& filePath);

    /// Load a file from the archive to memory and pass a pointer to it
    /// Warning: Returns nullptr if by any reason it can't load the file
    std::unique_ptr<char[]> loadToMemory(const std::string& assetname);

    /// Writes the contents of assetname to filename
    bool saveAsset(const std::string& assetname, const std::string& filename);

    /// Get the information of an asset in the examining archive
    bool findAssetInfo(const std::string& assetname, LoaderIMGFile& out);

    /// Get the information of an asset by its index
    const LoaderIMGFile& getAssetInfoByIndex(size_t index) const;

    /// Returns the number of asset files in the archive
    std::size_t getAssetCount() const;

    Version getVersion() const {
        return m_version;
    }

private:
    Version m_version = GTAIIIVC;  ///< Version of this IMG archive
    rwfs::path m_archive;  ///< Path to the archive being used (no extension)

    std::vector<LoaderIMGFile> m_assets;  ///< Asset info of the archive
};

#endif  // LoaderIMG_h__
