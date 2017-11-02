#pragma once
#ifndef _LOADERIMG_HPP_
#define _LOADERIMG_HPP_

#include <cstdint>
#include <iostream>
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
    enum Versions {
        GTAIIIVC,  ///< GTA III and GTA VC archives -- only this one is
                   ///implemented
        GTASA,
        GTAIV
    };

    /// Construct
    LoaderIMG();

    /// Load the structure of the archive
    /// Omit the extension in filename so both .dir and .img are loaded when
    /// appropriate
    bool load(const rwfs::path& filename);

    /// Load a file from the archive to memory and pass a pointer to it
    /// Warning: Please delete[] the memory in the end.
    /// Warning: Returns NULL (0) if by any reason it can't load the file
    char* loadToMemory(const std::string& assetname);

    /// Writes the contents of assetname to filename
    bool saveAsset(const std::string& assetname, const std::string& filename);

    /// Get the information of an asset in the examining archive
    bool findAssetInfo(const std::string& assetname, LoaderIMGFile& out);

    /// Get the information of an asset by its index
    const LoaderIMGFile& getAssetInfoByIndex(size_t index) const;

    /// Returns the number of asset files in the archive
    uint32_t getAssetCount() const;

private:
    Versions m_version;     ///< Version of this IMG archive
    uint32_t m_assetCount;  ///< Number of assets in the current archive
    rwfs::path m_archive;  ///< Path to the archive being used (no extension)

    std::vector<LoaderIMGFile> m_assets;  ///< Asset info of the archive
};

#endif  // LoaderIMG_h__
