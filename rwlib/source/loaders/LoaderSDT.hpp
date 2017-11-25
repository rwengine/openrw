#pragma once
#ifndef _LOADERSDT_HPP_
#define _LOADERSDT_HPP_

#include <cstdint>
#include <iostream>
#include <vector>
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}

// Rename some functions for older libavcodec/ffmpeg versions (e.g. Ubuntu Trusty)
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
#define av_frame_alloc  avcodec_alloc_frame
#define av_frame_free   avcodec_free_frame
#endif

/// \brief Points to one file within the archive
class LoaderSDTFile {
public:
    uint32_t offset;      // offset of audio file in sfx.raw
    uint32_t size;        // size of audio file in bytes
    uint32_t sampleRate;  // the speed of audio
    uint32_t loopStart;   /// loop start, where looping would begin relative to
                          /// audio file's position, 0 for beginning of audio
                          /// file
    uint32_t loopEnd;     /// where looping would end relative to audio file's
                          /// position, -1 for end of audio file
};

/**
    \class LoaderSDT
    \brief Parses the structure of GTA .SDT archives and loads the files in it
*/
class LoaderSDT {
public:
    /// Multiple versions of .SDT files
    enum Versions {
        GTA2,
        GTAIIIVC  ///< GTA III and GTA VC archives -- only this one is
                  ///implemented
    };

    /// Construct
    LoaderSDT();

    /// Load the structure of the archive
    /// Omit the extension in filename
    bool load(const std::string& filename);

    /// Load a file from the archive to memory and pass a pointer to it
    /// Warning: Please delete[] the memory in the end.
    /// Warning: Returns NULL (0) if by any reason it can't load the file
    char* loadToMemory(size_t index, bool asWave = true);

    AVFormatContext * loadSound(size_t index, bool asWave = true);


    /// Writes the contents of index to filename
    bool saveAsset(size_t index, const std::string& filename,
                   bool asWave = true);

    /// Get the information of an asset in the examining archive
    bool findAssetInfo(size_t index, LoaderSDTFile& out);

    /// Get the information of an asset by its index
    const LoaderSDTFile& getAssetInfoByIndex(size_t index) const;

    /// Returns the number of asset files in the archive
    uint32_t getAssetCount() const;

private:
    Versions m_version;     ///< Version of this SDT archive
    uint32_t m_assetCount;  ///< Number of assets in the current archive
    std::string m_archive;  ///< Path to the archive being used (no extension)

    std::vector<LoaderSDTFile> m_assets;  ///< Asset info of the archive
};

#endif  // LoaderSDT_h__
