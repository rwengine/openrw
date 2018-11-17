#include "loaders/LoaderIMG.hpp"

#include <boost/algorithm/string/predicate.hpp>
#include <cstdio>

#include "rw/debug.hpp"

bool LoaderIMG::load(const rwfs::path& filepath) {
    auto dirPath = filepath;
    dirPath.replace_extension(".dir");

    FILE* fp = fopen(dirPath.string().c_str(), "rb");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        unsigned long fileSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        std::size_t expectedCount = fileSize / 32;
        m_assets.resize(expectedCount);
        std::size_t actualCount =
            fread(&m_assets[0], sizeof(LoaderIMGFile), expectedCount, fp);

        if (expectedCount != actualCount) {
            m_assets.resize(actualCount);
            RW_ERROR("Error reading records in IMG archive");
        }

        fclose(fp);
        auto imgPath = filepath;
        imgPath.replace_extension(".img");
        m_archive = imgPath;
        return true;
    } else {
        return false;
    }
}

/// Get the information of a asset in the examining archive
bool LoaderIMG::findAssetInfo(const std::string& assetname,
                              LoaderIMGFile& out) {
    for (auto& asset : m_assets) {
        if (boost::iequals(asset.name, assetname)) {
            out = asset;
            return true;
        }
    }
    return false;
}

std::unique_ptr<char[]> LoaderIMG::loadToMemory(const std::string& assetname) {
    LoaderIMGFile assetInfo;
    bool found = findAssetInfo(assetname, assetInfo);

    if (!found) {
        RW_ERROR("Asset '" << assetname << "' not found!");
        return nullptr;
    }

    auto imgName = m_archive;

    FILE* fp = fopen(imgName.string().c_str(), "rb");
    if (fp) {
        auto raw_data = std::make_unique<char[]>(assetInfo.size * 2048);

        fseek(fp, assetInfo.offset * 2048, SEEK_SET);
        if (fread(raw_data.get(), 2048, assetInfo.size, fp) != assetInfo.size) {
            RW_ERROR("Error reading asset " << assetInfo.name);
        }

        fclose(fp);
        return raw_data;
    } else
        return nullptr;
}

/// Writes the contents of assetname to filename
bool LoaderIMG::saveAsset(const std::string& assetname,
                          const std::string& filename) {
    auto raw_data = loadToMemory(assetname);
    if (!raw_data)
        return false;

    FILE* dumpFile = fopen(filename.c_str(), "wb");
    if (dumpFile) {
        LoaderIMGFile asset;
        if (findAssetInfo(assetname, asset)) {
            fwrite(raw_data.get(), 2048, asset.size, dumpFile);
            printf("=> IMG: Saved %s to disk with filename %s\n",
                   assetname.c_str(), filename.c_str());
        }
        fclose(dumpFile);

        return true;
    } else {
        return false;
    }
}

/// Get the information of an asset by its index
const LoaderIMGFile& LoaderIMG::getAssetInfoByIndex(size_t index) const {
    return m_assets[index];
}

std::size_t LoaderIMG::getAssetCount() const {
    return m_assets.size();
}
