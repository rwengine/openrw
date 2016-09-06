#include <engine/GameData.hpp>
#include <engine/GameWorld.hpp>
#include <engine/GameState.hpp>
#include <loaders/LoaderIPL.hpp>
#include <loaders/LoaderDFF.hpp>
#include <loaders/LoaderIDE.hpp>
#include <loaders/LoaderCOL.hpp>
#include <data/ObjectData.hpp>
#include <data/WeaponData.hpp>
#include <script/SCMFile.hpp>
#include <data/Model.hpp>

#include <loaders/GenericDATLoader.hpp>
#include <loaders/LoaderGXT.hpp>
#include <loaders/BackgroundLoader.hpp>
#include <core/Logger.hpp>
#include <platform/FileIndex.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <boost/algorithm/string/predicate.hpp>

GameData::GameData(Logger* log, WorkContext* work, const std::string& path)
: datpath(path), logger(log), workContext(work), engine(nullptr)
{
}

GameData::~GameData()
{
	for(auto& m : models) {
		if(m.second->resource) {
			delete m.second->resource;
		}
	}
}

void GameData::load()
{
	index.indexGameDirectory(datpath);
	index.indexTree(datpath);

	loadIMG("models/gta3.img");
	/// @todo cuts.img files should be loaded differently to gta3.img
	loadIMG("anim/cuts.img");

	loadLevelFile("data/default.dat");
	loadLevelFile("data/gta3.dat");
	
	loadDFF("wheels.dff");
	loadDFF("weapons.dff");
	loadDFF("arrow.dff");
	loadTXD("particle.txd");
	loadTXD("icons.txd");
	loadTXD("hud.txd");
	loadTXD("fonts.txd");
	
	loadCarcols("data/carcols.dat");
	loadWeather("data/timecyc.dat");
	loadHandling("data/handling.cfg");
	loadWaterpro("data/waterpro.dat");
	loadWeaponDAT("data/weapon.dat");

	loadIFP("ped.ifp");
}

void GameData::loadLevelFile(const std::string& path)
{
	auto datpath = index.findFilePath(path);
	std::ifstream datfile(datpath.c_str());

	if(!datfile.is_open()) 
	{
		logger->error("Data", "Failed to open game file " + path);
	}
	else
	{
		for(std::string line, cmd; std::getline(datfile, line);)
		{
			if(line.size() == 0 || line[0] == '#') continue;
			#ifndef RW_WINDOWS
				line.erase(line.size()-1);
			#endif

			size_t space = line.find_first_of(' ');
			if(space != line.npos)
			{
				cmd = line.substr(0, space);
				if(cmd == "IDE")
				{
					auto path = line.substr(space+1);
					loadIDE(path);
				}
				else if(cmd == "SPLASH")
				{
					splash = line.substr(space+1);
				}
				else if(cmd == "COLFILE")
				{
					int zone  = atoi(line.substr(space+1,1).c_str());
					auto path = line.substr(space+3);
					loadCOL(zone, path);
				}
				else if(cmd == "IPL")
				{
					auto path = line.substr(space+1);
					loadIPL(path);
				}
				else if(cmd == "TEXDICTION")
				{
					auto path = line.substr(space+1);
					/// @todo improve TXD handling
					auto name = index.findFilePath(path).filename().native();
					std::transform(name.begin(), name.end(), name.begin(), ::tolower);
					loadTXD(name);
				}
			}
		}
	}
}

void GameData::loadIDE(const std::string& path)
{
	auto systempath = index.findFilePath(path).native();
	LoaderIDE idel;

	if(idel.load(systempath)) {
		objectTypes.insert(idel.objects.begin(), idel.objects.end());
	}
	else {
		logger->error("Data", "Failed to load IDE " + path);
	}
}

uint16_t GameData::findModelObject(const std::string model)
{
	auto defit = std::find_if(objectTypes.begin(), objectTypes.end(),
							  [&](const decltype(objectTypes)::value_type& d)
							  {
								  if(d.second->class_type == ObjectInformation::_class("OBJS"))
								  {
									  auto dat = static_cast<ObjectData*>(d.second.get());
									  return boost::iequals(dat->modelName, model);
								  }
								  return false;
							  });
	if( defit != objectTypes.end() ) return defit->first;
	return -1;
}

void GameData::loadCOL(const size_t zone, const std::string& name)
{
	RW_UNUSED(zone);

	LoaderCOL col;
	
	auto systempath = index.findFilePath(name).native();
	
	if(col.load(systempath)) {
		for( size_t i = 0; i < col.instances.size(); ++i ) {
			collisions[col.instances[i]->name] = std::move(col.instances[i]);
		}
	}
}

void GameData::loadIMG(const std::string& name)
{
	auto syspath = index.findFilePath(name).native();
	index.indexArchive(syspath);
}

void GameData::loadIPL(const std::string& path)
{
	auto systempath = index.findFilePath(path).native();
	iplLocations.insert({path, systempath});
}

bool GameData::loadZone(const std::string& path)
{
	LoaderIPL ipll;
	
	if( ipll.load(path)) {
		if( ipll.zones.size() > 0) {
			for(auto& z : ipll.zones) {
				zones.insert({z.name, z});
			}
			logger->info("Data", "Loaded " + std::to_string(ipll.zones.size()) + " zones from " + path);
			return true;
		}
	}
	else {
		logger->error("Data", "Failed to load zones from " + path);
	}
	
	return false;
}

enum ColSection {
	Unknown,
	COL,
	CAR,
	CAR3, ///> Used in GTASA, contains extra specular color
	CAR4  ///> Used in GTASA, contains quadruple colors
};

void GameData::loadCarcols(const std::string& path)
{
	auto syspath = index.findFilePath(path);
	std::ifstream fstream(syspath.c_str());
	
	std::string line;
	ColSection currentSection = Unknown;
	while( std::getline(fstream, line)) {
		if( line.substr(0, 1) == "#") { // Comment 
			continue; 
		}
		else if( currentSection == Unknown) {
			if( line.substr(0, 3) == "col") {
				currentSection = COL;
			}
			else if( line.substr(0, 3) == "car") {
				currentSection = CAR;
			}
		}
		else if( line.substr(0, 3) == "end") {
			currentSection = Unknown;
		}
		else {
			if( currentSection == COL) {
				std::string r, g, b;
				std::stringstream ss(line);
				
				if( std::getline(ss, r, ',') && std::getline(ss, g, ',') && std::getline(ss, b)) {
					vehicleColours.push_back(glm::u8vec3(
						atoi(r.c_str()),
						atoi(g.c_str()),
						atoi(b.c_str())
					));
				}
			}
			else if( currentSection == CAR) {
				std::string vehicle, p, s;
				std::stringstream ss(line);
				
				std::getline(ss, vehicle, ',');
				std::vector<std::pair<size_t, size_t>> colours;
				
				while( std::getline(ss, p, ',') && std::getline(ss, s, ',') ) {
					colours.push_back({ atoi(p.c_str()), atoi(s.c_str()) });
				}
				
				vehiclePalettes.insert({vehicle, colours});
			}
		}
	}
}

void GameData::loadWeather(const std::string &path)
{
	auto syspath = index.findFilePath(path).native();
	weatherLoader.load(syspath);
}

void GameData::loadHandling(const std::string& path)
{
	GenericDATLoader l;
	auto syspath = index.findFilePath(path).native();

	l.loadHandling(syspath, vehicleInfo);
}

SCMFile *GameData::loadSCM(const std::string &path)
{
	auto scm_h = index.openFilePath(path);
	SCMFile* scm = new SCMFile;
	scm->loadFile(scm_h->data, scm_h->length);
	scm_h.reset();
	return scm;
}

void GameData::loadGXT(const std::string &name)
{
	auto file = index.openFilePath(name);

	LoaderGXT loader;

	loader.load( texts, file );
}

void GameData::loadWaterpro(const std::string& path)
{
	auto syspath = index.findFilePath(path);
	std::ifstream ifstr(syspath.c_str(), std::ios_base::binary);
	
	if(ifstr.is_open()) {
		uint32_t numlevels;
		ifstr.read(reinterpret_cast<char*>(&numlevels), sizeof(uint32_t));
		ifstr.read(reinterpret_cast<char*>(&waterHeights), sizeof(float)*48);
		
		ifstr.seekg(0x03C4);
		ifstr.read(reinterpret_cast<char*>(&visibleWater), sizeof(char)*64*64);
		ifstr.read(reinterpret_cast<char*>(&realWater), sizeof(char)*128*128);
	}
}

void GameData::loadWater(const std::string& path)
{
	std::ifstream ifstr(path.c_str());
	
	std::string line;
	while( std::getline(ifstr, line)) {
		if( line[0] == ';') {
			continue;
		}
		
		std::stringstream ss(line);
		
		std::string a, b, c, d, e;
		float fa, fb, fc, fd, fe;
		
		if( std::getline(ss, a, ',') &&  std::getline(ss, b, ',') &&  std::getline(ss, c, ',') &&  std::getline(ss, d, ',') &&  std::getline(ss, e, ',')) {
			fa = atof(a.c_str());
			fb = atof(b.c_str());
			fc = atof(c.c_str());
			fd = atof(d.c_str());
			fe = atof(e.c_str());
			waterBlocks.push_back({
				fa,
				fb,
				fc,
				fd,
				fe
			});
		}
	}
}

void GameData::loadTXD(const std::string& name, bool async)
{
	if( loadedFiles.find(name) != loadedFiles.end() ) {
		return;
	}

	loadedFiles[name] = true;

	auto j = new LoadTextureArchiveJob(workContext, &index, textures, name);

	if( async ) {
		workContext->queueJob( j );
	}
	else {
		j->work();
		j->complete();
		delete j;
	}
}

void GameData::loadDFF(const std::string& name, bool async)
{
	auto realname = name.substr(0, name.size() - 4);
	if( models.find(realname) != models.end() ) {
		return;
	}

	// Before starting the job make sure the file isn't loaded again.
	loadedFiles.insert({name, true});

	models[realname] = ModelRef( new ResourceHandle<Model>(realname) );
	
	auto job = new BackgroundLoaderJob<Model, LoaderDFF> 
	{ workContext, &this->index, name, models[realname] };

	if( async ) {
		workContext->queueJob( job  );
	}
	else {
		job->work();
		job->complete();
		delete job;
	}

}

void GameData::loadIFP(const std::string &name)
{
	auto f = openFile(name);

	if(f)
	{
		LoaderIFP loader;
		if( loader.loadFromMemory(f->data) ) {
			animations.insert(loader.animations.begin(), loader.animations.end());
		}

	}
}

void GameData::loadDynamicObjects(const std::string& name)
{
	GenericDATLoader l;

	l.loadDynamicObjects(name, dynamicObjectData);
}

void GameData::loadWeaponDAT(const std::string &path)
{
	GenericDATLoader l;
	auto syspath = index.findFilePath(path).native();

	l.loadWeapons(syspath, weaponData);
}

bool GameData::loadAudioStream(const std::string &name)
{
	auto systempath = index.findFilePath("audio/" + name).native();
	
	if (engine->cutsceneAudio.length() > 0) {
		engine->sound.stopMusic(engine->cutsceneAudio);
	}

	if (engine->sound.loadMusic(name, systempath)) {
		engine->cutsceneAudio = name;
		return true;
	}

	return false;
}

bool GameData::loadAudioClip(const std::string& name, const std::string& fileName)
{
	auto systempath = index.findFilePath("audio/" + fileName).native();

	if (systempath.find(".mp3") != std::string::npos)
	{
		logger->error("Data", "MP3 Audio unsupported outside cutscenes");
		return false;
	}

	bool loaded = engine->sound.loadSound(name, systempath);

	if ( ! loaded) {
		logger->error("Data", "Error loading audio clip "+ systempath);
		return false;
	}

	engine->missionAudio = name;

	return true;
}

void GameData::loadSplash(const std::string &name)
{
	std::string lower(name);
	std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

	loadTXD(lower + ".txd", false);

	engine->state->currentSplash = lower;
}

FileHandle GameData::openFile(const std::string &name)
{
	auto file = index.openFile(name);
	if( file == nullptr )
	{
		logger->error("Data", "Unable to open file: " + name);
	}
	return file;
}

int GameData::getWaterIndexAt(const glm::vec3 &ws) const
{
	auto wX = (int) ((ws.x + WATER_WORLD_SIZE/2.f) / (WATER_WORLD_SIZE/WATER_HQ_DATA_SIZE));
	auto wY = (int) ((ws.y + WATER_WORLD_SIZE/2.f) / (WATER_WORLD_SIZE/WATER_HQ_DATA_SIZE));

	if( wX >= 0 && wX < WATER_HQ_DATA_SIZE && wY >= 0 && wY < WATER_HQ_DATA_SIZE ) {
		int i = (wX*WATER_HQ_DATA_SIZE) + wY;
		return engine->data->realWater[i];
	}
	return 0;
}

float GameData::getWaveHeightAt(const glm::vec3 &ws) const
{
	return (1+sin(engine->getGameTime() + (ws.x + ws.y) * WATER_SCALE)) * WATER_HEIGHT;
}

bool GameData::isValidGameDirectory(const std::string& path)
{
	if(path.empty())
	{
		return false;
	}
	
	LoaderIMG i;
	if(! i.load(path + "/models/gta3.img") )
	{
		return false;
	}
	
	return true;
}

