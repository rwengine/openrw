#include "loaders/LoaderSDT.hpp"

#include <cstdio>
#include <cstring>
#include <string>

#include "rw/debug.hpp"

bool LoaderSDT::load(const rwfs::path& sdtPath, const rwfs::path& rawPath) {
    const auto sdtName = sdtPath.string();
    const auto rawName = rawPath.string();

    FILE* fp = fopen(sdtName.c_str(), "rb");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        unsigned long fileSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        size_t expectedCount = fileSize / 20;
        m_assets.resize(expectedCount);

        size_t actualCount =
            fread(&m_assets[0], sizeof(LoaderSDTFile), expectedCount, fp);

        if (expectedCount != actualCount) {
            m_assets.resize(actualCount);
            RW_ERROR("Error reading records in SDT archive");
        }

        fclose(fp);
        m_archive = rawName;
        return true;
    } else {
        RW_ERROR("Error cannot open " << sdtName);
        return false;
    }
}

/// Get the information of a asset in the examining archive
bool LoaderSDT::findAssetInfo(size_t index, LoaderSDTFile& out) {
    if (index < m_assets.size()) {
        out = m_assets[index];
        return true;
    }
    return false;
}

std::unique_ptr<char[]> LoaderSDT::loadToMemory(size_t index, bool asWave) {
    bool found = findAssetInfo(index, assetInfo);

    if (!found) {
        RW_ERROR("Asset " << std::to_string(index) << " not found!");
        return nullptr;
    }

    std::string rawName = m_archive;

    FILE* fp = fopen(rawName.c_str(), "rb");
    if (fp) {
        std::unique_ptr<char[]> raw_data;
        char* sample_data;
        if (asWave) {
            raw_data =
                std::make_unique<char[]>(sizeof(WaveHeader) + assetInfo.size);

            auto header = reinterpret_cast<WaveHeader*>(raw_data.get());
            memcpy(header->chunkId, "RIFF", 4);
            header->chunkSize = sizeof(WaveHeader) - 8 + assetInfo.size;
            memcpy(header->format, "WAVE", 4);
            memcpy(header->fmt.id, "fmt ", 4);
            header->fmt.size = sizeof(WaveHeader::fmt) - 8;
            header->fmt.audioFormat = 1;  // PCM
            header->fmt.numChannels = 1;  // Mono
            header->fmt.sampleRate = assetInfo.sampleRate;
            header->fmt.byteRate = assetInfo.sampleRate * 2;
            header->fmt.blockAlign = 2;
            header->fmt.bitsPerSample = 16;
            memcpy(header->data.id, "data", 4);
            header->data.size = assetInfo.size;

            sample_data = raw_data.get() + sizeof(WaveHeader);
        } else {
            raw_data = std::make_unique<char[]>(assetInfo.size);
            sample_data = raw_data.get();
        }

        fseek(fp, assetInfo.offset, SEEK_SET);
        if (fread(sample_data, 1, assetInfo.size, fp) != assetInfo.size) {
            RW_ERROR("Error reading asset " << std::to_string(index));
        }

        fclose(fp);
        return raw_data;
    } else
        return nullptr;
}

/// Writes the contents of assetname to filename
bool LoaderSDT::saveAsset(size_t index, const std::string& filename,
                          bool asWave) {
    auto raw_sound = loadToMemory(index, asWave);
    if (!raw_sound)
        return false;

    FILE* dumpFile = fopen(filename.c_str(), "wb");
    if (dumpFile) {
        if (findAssetInfo(index, assetInfo)) {
            fwrite(raw_sound.get(), 1,
                   assetInfo.size + (asWave ? sizeof(WaveHeader) : 0),
                   dumpFile);
            printf("=> SDT: Saved %zu to disk with filename %s\n", index,
                   filename.c_str());
        }
        fclose(dumpFile);

        return true;
    } else {
        return false;
    }
}

const LoaderSDTFile& LoaderSDT::getAssetInfoByIndex(size_t index) const {
    return m_assets[index];
}

size_t LoaderSDT::getAssetCount() const {
    return m_assets.size();
}

LoaderSDT::Version LoaderSDT::getVersion() const {
    return m_version;
}
