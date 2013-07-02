#include <renderwure/engine/GTAData.hpp>
#include <renderwure/loaders/LoaderIPL.hpp>
#include <renderwure/loaders/LoaderDFF.hpp>

#include <iostream>
#include <fstream>
#include <bits/algorithmfwd.h>

GTAData::GTAData(const std::string& path)
: datpath(path)
{

}

void GTAData::load()
{
	std::ifstream datfile((datpath+"/data/gta3.dat").c_str());
	
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
					loadIPL(line.substr(space+1));
				}
			}
		}
	}
}

void GTAData::loadIDE(const std::string& name)
{
	std::cout << "IDE File " << name << std::endl;
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
				if (filetype == "txd") 
				{
					char *file = imgLoader.loadToMemory(filename);
					if(file) {
						textureLoader.loadFromMemory(file);
					}
				}
				else if(filetype == "dff")
				{
					std::string modelname = filename.substr(0, filename.size() - 4);
					char *file = imgLoader.loadToMemory(filename);
					if(file)
					{
						LoaderDFF dffLoader;
						models[modelname] = std::move(dffLoader.loadFromMemory(file));
						delete[] file;
					}
				}
				else
				{
					fileLocations.insert({ filename, { true, archivePath }});
				}
			}
		}
		std::cout << "Archive loaded" << std::endl;
		archives.insert({name, imgLoader});
		std::cout << "Archive copied" << std::endl;
	}
}

char* GTAData::loadFile(const std::string& name)
{
	auto i = fileLocations.find(name);
	if(i != fileLocations.end())
	{
		if(i->second.archived)
		{
			// Find the archive
			auto ai = archives.find(i->second.path);
			if(ai != archives.end())
			{
				return ai->second.loadToMemory(name);
			}
		}
		else
		{
			std::cerr << "Cannot load unarchived files yet" << std::endl;
		}
	}
	
	return nullptr;
}

void GTAData::loadIPL(const std::string& name)
{
	std::string lowername = name;
	for(size_t t = 0; t < lowername.size(); ++t)
	{
		lowername[t] = tolower(lowername[t]);
	}
	iplLocations.insert({lowername, datpath + "/" + lowername});
}
