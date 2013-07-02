#include <renderwure/engine/GTAData.hpp>
#include <renderwure/loaders/LoaderIPL.hpp>
#include <renderwure/loaders/LoaderDFF.hpp>
#include <renderwure/loaders/LoaderIDE.hpp>

#include <iostream>
#include <fstream>
#include <bits/algorithmfwd.h>

#include <dirent.h>
#include <sys/types.h>

// Hacky functtion to get around the fact that none of the files are the right case
std::string findFileRealCase(const std::string& lowerPath) {
#ifdef __unix__
	size_t lastSlash = lowerPath.find_last_of("/");
	std::string fileDir = ".";
	if(lastSlash != lowerPath.npos) {
		fileDir = lowerPath.substr(0, lastSlash);
	}
	else {
		lastSlash = 0;
	}
	
	DIR* dp = opendir(fileDir.c_str());
	dirent* ep;
	if( dp != NULL) {
		while( ep = readdir(dp)) {
			std::string realName(ep->d_name);
			std::string lowerRealName = realName;
			for( size_t t = 0; t < lowerRealName.size(); ++t) {
				lowerRealName[t] = tolower(lowerRealName[t]);
			}
			if(lowerRealName == lowerPath.substr(lastSlash+1)) {
				return fileDir + "/" + realName;
			}
		}
		
		closedir(dp);
	}
	
	return lowerPath;
	
#else 
	// We'll just have to assume this means Windows for now.
	return lowerPath;
#endif
}

// Yet another hack function to fix these paths
std::string fixPath(std::string path) {
	for( size_t t = 0; t < path.size(); ++t) {
		path[t] = tolower(path[t]);
		if(path[t] == '\\') {
			path[t] = '/';
		}
	}
	return path;
}


GTAData::GTAData(const std::string& path)
: datpath(path)
{

}

void GTAData::load()
{
	parseDAT(datpath+"/data/default.dat");
	parseDAT(datpath+"/data/gta3.dat");
}

void GTAData::parseDAT(const std::string& path)
{
	std::ifstream datfile(path.c_str());
	
	if(!datfile.is_open()) 
	{
		std::cerr << "Failed to open gta.dat" << std::endl;
	}
	else
	{
		for(std::string line, cmd; std::getline(datfile, line);)
		{
			if(line.size() == 0 || line[0] == '#') continue;
			line.erase(line.size()-1);
			
			size_t space = line.find_first_of(' ');
			if(space != line.npos)
			{
				cmd = line.substr(0, space);
				if(cmd == "IDE")
				{
					loadIDE(line.substr(space+1));
				}
				else if(cmd == "SPLASH")
				{
					splash = line.substr(space+1);
				}
				else if(cmd == "COLFILE")
				{
					int zone  = atoi(line.substr(space+1,1).c_str());
					std::string file = line.substr(space+3);
					loadCOL(zone, file);
				}
				else if(cmd == "IPL")
				{
					std::string fixedpath = fixPath(line.substr(space+1));
					fixedpath = findFileRealCase(datpath + "/" + fixedpath);
					loadIPL(fixedpath.substr((datpath+"/").size()));
				}
				else if(cmd == "TEXDICTION") 
				{
					std::string texpath = line.substr(space+1);
					for( size_t t = 0; t < texpath.size(); ++t) {
						texpath[t] = tolower(texpath[t]);
						if(texpath[t] == '\\') {
							texpath[t] = '/';
						}
					}
					texpath = findFileRealCase(datpath + "/" + texpath);
					std::string texname = texpath.substr(texpath.find_last_of("/")+1);
					fileLocations.insert({ texname, { false, texpath }});
				}
			}
		}
	}
}

void GTAData::loadIDE(const std::string& name)
{
	std::string lowername = name;
	for(size_t t = 0; t < lowername.size(); ++t)
	{
		lowername[t] = tolower(lowername[t]);
		if(lowername[t] == '\\') {
			lowername[t] = '/';
		}
	}
	
	ideLocations.insert({lowername, datpath+"/"+lowername});
}

void GTAData::loadCOL(const size_t zone, const std::string& name)
{
	std::cout << "COL File " << name << " for zone " << zone << std::endl;
}

void GTAData::loadIMG(const std::string& name)
{
	LoaderIMG imgLoader; 
	std::string archivePath = datpath + name;

	if (imgLoader.load(archivePath)) {
		for (int i = 0; i < imgLoader.getAssetCount(); i++) {
			auto &asset = imgLoader.getAssetInfoByIndex(i);

			std::string filename = asset.name;
			
			if(asset.size == 0)
			{
				std::cerr << "Asset " << filename << " has no size, ignoring." << std::endl;
			}
			else
			{
				// Load TXDs immediatley
				auto filetype = filename.substr(filename.size() - 3);
				for(size_t t = 0; t < filetype.size(); ++t)
				{
					filetype[t] = tolower(filetype[t]);
				}
				fileLocations.insert({ filename, { true, archivePath }});
			}
		}
		archives.insert({archivePath, imgLoader});
	}
}

void GTAData::loadTXD(const std::string& name) 
{
	if( loadedFiles.find(name) != loadedFiles.end()) {
		return;
	}
	
	char* file = loadFile(name);
	if(file) {
		textureLoader.loadFromMemory(file);
		delete[] file;
	}
}

void GTAData::loadDFF(const std::string& name)
{
	if( loadedFiles.find(name) != loadedFiles.end()) {
		return;
	}
	
	char *file = loadFile(name);
	if(file)
	{
		LoaderDFF dffLoader;
		models[name.substr(0, name.size() - 4)] = std::move(dffLoader.loadFromMemory(file));
		delete[] file;
	}
}

char* GTAData::loadFile(const std::string& name)
{
	if( loadedFiles.find(name) != loadedFiles.end()) {
		std::cerr << "File " << name << " already loaded!" << std::endl;
		return nullptr;
	}
	
	auto i = fileLocations.find(name);
	if(i != fileLocations.end())
	{
		if(i->second.archived)
		{
			// Find the archive
			auto ai = archives.find(i->second.path);
			if(ai != archives.end())
			{
				loadedFiles[name] = true;
				return ai->second.loadToMemory(name);
			}
			else 
			{
				std::cerr << "Archive not found " << i->second.path << std::endl;
			}
		}
		else
		{
			std::ifstream dfile(i->second.path);
			if ( ! dfile.is_open()) {
				std::cerr << "Error opening file " << i->second.path << std::endl;
				return nullptr;
			}

			dfile.seekg(0, std::ios_base::end);
			size_t length = dfile.tellg();
			dfile.seekg(0);
			char *data = new char[length];
			dfile.read(data, length);
			
			loadedFiles[name] = true;
			
			return data;
		}
	}
	else 
	{
		std::cerr << "Unable to locate file " << name << std::endl;
	}
	
	return nullptr;
}

void GTAData::loadIPL(const std::string& name)
{
	iplLocations.insert({name, datpath + "/" + name});
}
