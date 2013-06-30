#include "gtadata.h"
#include "LoaderIPL.h"
#include <iostream>
#include <fstream>

GTAData::GTAData(const std::string& path)
: datpath(path)
{

}

void GTAData::load()
{
	std::ifstream datfile(datpath.c_str());
	
	std::string line, cmd;
	while(!datfile.eof())
	{
		std::getline(datfile, line);
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

void GTAData::loadIDE(const std::string& name)
{
	std::cout << "IDE File " << name << std::endl;
}

void GTAData::loadCOL(const size_t zone, const std::string& name)
{
	std::cout << "COL File " << name << " for zone " << zone << std::endl;
}

void GTAData::loadIPL(const std::string& name)
{
	LoaderIPL ipll;
	
	if(ipll.load(name))
	{
		// wait for it..
	}
	else
	{
		std::cerr << "Failed to load IPL: " << name << std::endl;
	}
}
