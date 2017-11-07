#include <algorithm>
#include <fstream>
#include <loaders/LoaderIMG.hpp>
#include <platform/FileIndex.hpp>

void FileIndex::indexGameDirectory(const rwfs::path& base_path) {
    gamedatapath_ = base_path;

    for (const rwfs::path& path :
         rwfs::recursive_directory_iterator(base_path)) {
        if (!rwfs::is_regular_file(path)) {
            continue;
        }

        std::string name = path.string();
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        filesystemfiles_[name] = path;
    }
}

FileHandle FileIndex::openFilePath(const std::string& file_path) {
    auto datapath = findFilePath(file_path);
    std::ifstream dfile(datapath.string(),
                        std::ios_base::binary | std::ios_base::ate);
    if (!dfile.is_open()) {
        throw std::runtime_error("Unable to open file: " + file_path);
    }

    auto length = dfile.tellg();
    dfile.seekg(0);
    auto data = new char[length];
    dfile.read(data, length);

    return std::make_shared<FileContentsInfo>(data, length);
}

void FileIndex::indexTree(const rwfs::path& root) {
    for (const rwfs::path& path : rwfs::recursive_directory_iterator(root)) {
        if (!rwfs::is_regular_file(path)) {
            continue;
        }

        std::string directory = path.parent_path().string();
        std::string realName = path.filename().string();
        std::string lowerName = realName;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(),
                       ::tolower);
        files[lowerName] = {lowerName, realName, directory, ""};
    }
}

void FileIndex::indexArchive(const std::string& archive) {
    // Split directory from archive name
    auto archive_path = rwfs::path(archive);
    auto directory = archive_path.parent_path();
    auto archive_basename = archive_path.filename();
    auto archive_full_path = directory / archive_basename;

    LoaderIMG img;
    if (!img.load(archive_full_path.string())) {
        throw std::runtime_error("Failed to load IMG archive: " +
                                 archive_full_path.string());
    }

    std::string lowerName;
    for (size_t i = 0; i < img.getAssetCount(); ++i) {
        auto& asset = img.getAssetInfoByIndex(i);

        if (asset.size == 0) continue;

        lowerName = asset.name;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(),
                       ::tolower);

        files[lowerName] = {lowerName, asset.name, directory.string(),
                            archive_basename.string()};
    }
}

FileHandle FileIndex::openFile(const std::string& filename) {
    auto iterator = files.find(filename);
    if (iterator == files.end()) {
        return nullptr;
    }

    IndexData& f = iterator->second;
    bool isArchive = !f.archive.empty();

    auto fsName = f.directory + "/" + f.originalName;

    char* data = nullptr;
    size_t length = 0;

    if (isArchive) {
        fsName = f.directory + "/" + f.archive;

        LoaderIMG img;

        if (!img.load(fsName)) {
            throw std::runtime_error("Failed to load IMG archive: " + fsName);
        }

        LoaderIMGFile file;
        if (img.findAssetInfo(f.originalName, file)) {
            length = file.size * 2048;
            data = img.loadToMemory(f.originalName);
        }
    } else {
        std::ifstream dfile(fsName.c_str(), std::ios_base::binary);
        if (!dfile.is_open()) {
            throw std::runtime_error("Unable to open file: " + fsName);
        }

        dfile.seekg(0, std::ios_base::end);
        length = dfile.tellg();
        dfile.seekg(0);
        data = new char[length];
        dfile.read(data, length);
    }

    if (data == nullptr) {
        return nullptr;
    }

    return std::make_shared<FileContentsInfo>(data, length);
}
