#include "engine/GTAEngine.h"
#include "loaders/LoaderIPL.h"

GTAEngine::GTAEngine(const std::string& path)
: gameData(path)
{
	
}

bool GTAEngine::load()
{
	gameData.load();
	
	return true;
}

bool GTAEngine::loadItems(const std::string& name)
{
	auto i = gameData.iplLocations.find(name);
	std::string path = name;
	
	if(i != gameData.iplLocations.end())
	{
		path = i->second;
	}
	else
	{
		std::cout << "IPL not pre-listed" << std::endl;
	}
	
	LoaderIPL ipll;

	if(ipll.load(path))
	{
		instances.insert(instances.end(), ipll.m_instances.begin(), ipll.m_instances.end());
		itemCentroid += ipll.centroid;
		return true;
	}
	else
	{
		std::cerr << "Failed to load IPL: " << path << std::endl;
		return false;
	}
	
	return false;
}