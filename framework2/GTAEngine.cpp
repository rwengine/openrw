#include <renderwure/engine/GTAEngine.hpp>
#include <renderwure/loaders/LoaderIPL.hpp>
#include <renderwure/loaders/LoaderIDE.hpp>

GTAEngine::GTAEngine(const std::string& path)
: itemCount(0), gameData(path), gameTime(0.f)
{
	
}

bool GTAEngine::load()
{
	gameData.load();
	
	// Loade all of the IDEs.
	for(std::map<std::string, std::string>::iterator it = gameData.ideLocations.begin();
		it != gameData.ideLocations.end();
		++it) {
		defineItems(it->second);
	}
	
	return true;
}

void GTAEngine::logInfo(const std::string& info)
{
	log.push({LogEntry::Info, gameTime, info});
}

void GTAEngine::logError(const std::string& error)
{
	log.push({LogEntry::Error, gameTime, error});
}

bool GTAEngine::defineItems(const std::string& name)
{
	auto i = gameData.ideLocations.find(name);
	std::string path = name;
	
	if( i != gameData.ideLocations.end()) {
		path = i->second;
	}
	else {
		std::cout << "IDE not pre-listed" << std::endl;
	}
	
	LoaderIDE idel;
	
	if(idel.load(path)) {
		for( size_t o = 0; o < idel.OBJSs.size(); ++o) {
			objectTypes.insert({
				idel.OBJSs[o].ID, 
				std::shared_ptr<LoaderIDE::OBJS_t>(new LoaderIDE::OBJS_t(idel.OBJSs[o]))
			});
		}
	}
	else {
		std::cerr << "Failed to load IDE " << path << std::endl;
	}
	
	return false;
}

bool GTAEngine::placeItems(const std::string& name)
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
		// Find the object.
		for( size_t i = 0; i < ipll.m_instances.size(); ++i) {
			LoaderIPLInstance& inst = ipll.m_instances[i];
			auto oi = objectTypes.find(inst.id);
			if( oi != objectTypes.end()) {
				objectInstances.push_back({ inst, oi->second });
			}
			else {
				std::cerr << "No object for instance " << inst.id << " (" << path << ")" << std::endl;
			}
		}
		itemCentroid += ipll.centroid;
		itemCount += ipll.m_instances.size();
		return true;
	}
	else
	{
		std::cerr << "Failed to load IPL: " << path << std::endl;
		return false;
	}
	
	return false;
}