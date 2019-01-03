#ifndef _LIBRW_LOADERSDT_HPP_
#define _LIBRW_LOADERSDT_HPP_

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

typedef struct {
    char chunkId[4];
    uint32_t chunkSize;
    char format[4];
    struct {
        char id[4];
        uint32_t size;
        uint16_t audioFormat;
        uint16_t numChannels;
        uint32_t sampleRate;
        uint32_t byteRate;
        uint16_t blockAlign;
        uint16_t bitsPerSample;
    } fmt;
    struct {
        char id[4];
        uint32_t size;
    } data;
} WaveHeader;

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
    enum Version {
        GTA2,
        GTAIIIVC  ///< GTA III and GTA VC archives -- only this one is
                  ///implemented
    };

    /// Construct
    LoaderSDT() = default;

    /// Destructor
    ~LoaderSDT() = default;

    /// Load the structure of the archive
    bool load(const std::filesystem::path& sdtPath, const std::filesystem::path& rawPath);

    /// Load a file from the archive to memory and pass a pointer to it
    /// Warning: Returns nullptr if by any reason it can't load the file
    std::unique_ptr<char[]> loadToMemory(size_t index, bool asWave = true);

    /// Writes the contents of index to filename
    bool saveAsset(size_t index, const std::string& filename,
                   bool asWave = true);

    /// Get the information of an asset in the examining archive
    bool findAssetInfo(size_t index, LoaderSDTFile& out);

    /// Get the information of an asset by its index
    const LoaderSDTFile& getAssetInfoByIndex(size_t index) const;

    /// Returns the number of asset files in the archive
    size_t getAssetCount() const;

    Version getVersion() const;
    LoaderSDTFile assetInfo{};
private:
    Version m_version{GTAIIIVC};      ///< Version of this SDT archive
    std::string m_archive;  ///< Path to the archive being used (no extension)
    std::vector<LoaderSDTFile> m_assets;  ///< Asset info of the archive
};

#endif  // LoaderSDT_h__
