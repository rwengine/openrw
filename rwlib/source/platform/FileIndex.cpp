#include <algorithm>
#include <fstream>
#include <boost/range/iterator_range.hpp>
#include <platform/FileIndex.hpp>
#include <loaders/LoaderIMG.hpp>

using namespace boost::filesystem;

void FileIndex::indexGameDirectory(const fs::path& base_path)
{
	gamedatapath_ = base_path;

	for(const path& entry : boost::make_iterator_range(recursive_directory_iterator(base_path), {})) {
		if(is_regular_file(entry)) {
			std::string name = entry.native();
			std::transform(name.begin(), name.end(), name.begin(), ::tolower);

			filesystemfiles_[name] = entry;
		}
	}
}

FileHandle FileIndex::openFilePath(const std::string &file_path)
{
	auto datapath = findFilePath(file_path);
	std::ifstream dfile(datapath.c_str(), std::ios_base::binary | std::ios_base::ate);
	if ( ! dfile.is_open()) {
		throw std::runtime_error("Unable to open file: " + file_path);
	}

	auto length = dfile.tellg();
	dfile.seekg(0);
	auto data = new char[length];
	dfile.read(data, length);

	return std::make_shared<FileContentsInfo> ( data, length );
}

void FileIndex::indexTree(const std::string& root)
{
	for(const path& entry : boost::make_iterator_range(recursive_directory_iterator(root), {})) {
		std::string directory = entry.parent_path().string();
		std::string realName = entry.filename().string();
		std::string lowerName = realName;
		std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);

		if(is_regular_file(entry)) {
			files[lowerName] = {lowerName, realName, directory, ""};
		}
	}
}

void FileIndex::indexArchive(const std::string& archive)
{
	// Split directory from archive name
	path archive_path = path(archive);
	path directory = archive_path.parent_path();
	path archive_basename = archive_path.filename();
	path archive_full_path = directory/archive_basename;
	
	LoaderIMG img;	
	if(!img.load(archive_full_path.string())) {
		throw std::runtime_error("Failed to load IMG archive: " + archive_full_path.string());
	}
	
	std::string lowerName;
	for( size_t i = 0; i < img.getAssetCount(); ++i ) {
		auto& asset = img.getAssetInfoByIndex(i);
		
		if( asset.size == 0 ) continue;
		
		lowerName = asset.name;
		std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
		
		files[lowerName] = {lowerName, asset.name, directory.string(), archive_basename.string()};
	}
}

FileHandle FileIndex::openFile(const std::string& filename)
{
	auto iterator = files.find( filename );
	if( iterator == files.end() ) {
		return nullptr;
	}
	
	IndexData& f = iterator->second;
	bool isArchive = !f.archive.empty();
	
	auto fsName = f.directory + "/" + f.originalName;
	
	char* data = nullptr;
	size_t length = 0;
	
	if( isArchive ) {
		fsName = f.directory + "/" + f.archive;
		
		LoaderIMG img;
		
		if( ! img.load(fsName) ) {
			throw std::runtime_error("Failed to load IMG archive: " + fsName);
		}
		
		LoaderIMGFile file;
		if( img.findAssetInfo(f.originalName, file) ) {
			length = file.size * 2048;
			data = img.loadToMemory(f.originalName);
		}
	}
	else {
		std::ifstream dfile(fsName.c_str(), std::ios_base::binary);
		if ( ! dfile.is_open()) {
			throw std::runtime_error("Unable to open file: " + fsName);
		}

		dfile.seekg(0, std::ios_base::end);
		length = dfile.tellg();
		dfile.seekg(0);
		data = new char[length];
		dfile.read(data, length);
	}
	
	if( data == nullptr ) {
		return nullptr;
	}
	
	return FileHandle( new FileContentsInfo{ data, length } );
}
