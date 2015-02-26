#include <core/FileIndex.hpp>
#include <loaders/LoaderIMG.hpp>

#include <algorithm>
#include <fstream>
#include <dirent.h>
#include <sys/types.h>

void FileIndex::indexDirectory(const std::string& directory)
{
	DIR* dp = opendir(directory.c_str());
	dirent* ep;
	std::string realName, lowerName;
	if ( dp == NULL ) {
		throw std::runtime_error("Unable to open directory: " + directory);
	}
	while( (ep = readdir(dp)) )
	{
		realName = ep->d_name;
		lowerName = realName;
		std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
		if ( ep->d_type == DT_REG )
		{
			files[ lowerName ] = {
				lowerName,
				realName,
				directory,
				""
			};
		}
	}
}

void FileIndex::indexArchive(const std::string& archive)
{
	// Split directory from archive name
	auto slash = archive.find_last_of('/');
	auto directory = archive.substr(0, slash);
	auto archivebasename = archive.substr(slash+1);
	auto archivepath = directory + "/" + archivebasename;
	
	LoaderIMG img;
	
	if( ! img.load( archivepath ) )
	{
		throw std::runtime_error("Failed to load IMG archive: " + archivepath);
	}
	
	std::string lowerName;
	for( size_t i = 0; i < img.getAssetCount(); ++i )
	{
		auto& asset = img.getAssetInfoByIndex(i);
		
		if( asset.size == 0 ) continue;
		
		lowerName = asset.name;
		std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
		
		files[ lowerName ] = {
			lowerName,
			asset.name,
			directory,
			archivebasename
		};
	}
}

bool FileIndex::findFile(const std::string& filename, FileIndex::IndexData& filedata)
{
	auto iterator = files.find( filename );
	if( iterator == files.end() )
	{
		return false;
	}
	
	filedata = iterator->second;
	
	return true;
}

FileHandle FileIndex::openFile(const std::string& filename)
{
	auto iterator = files.find( filename );
	if( iterator == files.end() )
	{
		return nullptr;
	}
	
	IndexData& f = iterator->second;
	bool isArchive = !f.archive.empty();
	
	auto fsName = f.directory + "/" + f.originalName;
	
	char* data = nullptr;
	size_t length = 0;
	
	if( isArchive )
	{
		fsName = f.directory + "/" + f.archive;
		
		LoaderIMG img;
		
		if( ! img.load(fsName) )
		{
			throw std::runtime_error("Failed to load IMG archive: " + fsName);
		}
		
		LoaderIMGFile file;
		if( img.findAssetInfo(f.originalName, file) )
		{
			length = file.size * 2048;
			data = img.loadToMemory(f.originalName);
		}
	}
	else
	{
		std::ifstream dfile(fsName.c_str());
		if ( ! dfile.is_open()) {
			throw std::runtime_error("Unable to open file: " + fsName);
		}

		dfile.seekg(0, std::ios_base::end);
		size_t length = dfile.tellg();
		dfile.seekg(0);
		char *data = new char[length];
		dfile.read(data, length);
	}
	
	return FileHandle( new FileContentsInfo{ data, length } );
}
