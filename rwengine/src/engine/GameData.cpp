#include <engine/GameData.hpp>
#include <engine/GameWorld.hpp>
#include <loaders/LoaderIPL.hpp>
#include <loaders/LoaderDFF.hpp>
#include <loaders/LoaderIDE.hpp>
#include <render/TextureAtlas.hpp>
#include <loaders/LoaderCOL.hpp>
#include <data/ObjectData.hpp>
#include <data/WeaponData.hpp>
#include <script/SCMFile.hpp>
#include <render/Model.hpp>

#include <loaders/GenericDATLoader.hpp>
#include <loaders/LoaderGXT.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include <dirent.h>
#include <sys/types.h>

/**
 * Finds the 'real' case for a path, to get around the fact that Rockstar's data is usually the wrong case.
 * @param base The base of the path to start looking from.
 * @param path the lowercase path.
 */
std::string findPathRealCase(const std::string& base, const std::string& path) 
{
#ifdef __unix__
	size_t endslash = path.find("/");
	bool isDirectory = true;
	if(endslash == path.npos) {
		isDirectory = false;
	}
	std::string orgFileName = isDirectory ? path.substr(0, endslash) : path;
	std::transform(orgFileName.begin(), orgFileName.end(), orgFileName.begin(), ::tolower);
	std::string realName;
	
	// Open the current "base" path (i.e. the real path)
	DIR* dp = opendir(base.c_str());
	dirent* ep;
	
	if( dp != NULL) {
		while( (ep = readdir(dp)) ) {
			realName = ep->d_name;
			std::string lowerRealName = realName;
			std::transform(lowerRealName.begin(), lowerRealName.end(), lowerRealName.begin(), ::tolower);
			if( lowerRealName == orgFileName) {
				closedir(dp);
				if( isDirectory) {
					return findPathRealCase(base + "/" + realName, path.substr(endslash+1));
				}
				else {
					return base + "/" + realName;
				}
			}
		}
	}
	
	return "";
#else 
	// Is anything other than Windows likely to fall here?
	return path;
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


GameData::GameData(const std::string& path)
: datpath(path), engine(nullptr)
{

}

GameData::~GameData()
{
	for(auto& m : models) {
		if(m.second->model) {
			delete m.second->model;
		}
		delete m.second;
	}
}

void GameData::load()
{
	index.indexTree(datpath);
	
	parseDAT(datpath+"/data/default.dat");
	parseDAT(datpath+"/data/gta3.dat");
	
	loadDFF("wheels.dff");
	loadDFF("weapons.dff");
	loadDFF("arrow.dff");
	loadTXD("particle.txd");
	loadTXD("hud.txd");
	loadTXD("fonts.txd");
	
	loadCarcols(datpath+"/data/carcols.dat");
	loadWeather(datpath+"/data/timecyc.dat");
	loadHandling(datpath+"/data/handling.cfg");
	loadWaterpro(datpath+"/data/waterpro.dat");
	loadWater(datpath+"/data/water.dat");
	loadWeaponDAT(datpath+"/data/weapon.dat");

	loadIFP("ped.ifp");
}

void GameData::parseDAT(const std::string& path)
{
	std::ifstream datfile(path.c_str());
	
	if(!datfile.is_open()) 
	{
		engine->logger.error("Data", "Failed to open game file " + path);
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
					fixedpath = findPathRealCase(datpath, fixedpath);
					loadIPL(fixedpath);
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
					std::string texname = texpath.substr(texpath.find_last_of("/")+1);
					loadTXD(texname);
				}
			}
		}
	}
}

void GameData::loadIDE(const std::string& name)
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

void GameData::loadCOL(const size_t zone, const std::string& name)
{
	LoaderCOL col;
	
	std::string realPath = fixPath(name);
	realPath = findPathRealCase(datpath, realPath);
	
	if(col.load(realPath)) {
		for( size_t i = 0; i < col.instances.size(); ++i ) {
			collisions[col.instances[i]->name] = std::move(col.instances[i]);
		}
	}
}

void GameData::loadIMG(const std::string& name)
{
	index.indexArchive(datpath + name);
}

void GameData::loadIPL(const std::string& name)
{
	iplLocations.insert({name, name});
}

bool GameData::loadZone(const std::string& path)
{
	LoaderIPL ipll;
	
	if( ipll.load(path)) {
		if( ipll.zones.size() > 0) {
			for(auto& z : ipll.zones) {
				zones.insert({z.name, z});
			}
			engine->logger.info("Data", "Loaded " + std::to_string(ipll.zones.size()) + " zones from " + path);
			return true;
		}
	}
	else {
		engine->logger.error("Data", "Failed to load zones from " + path);
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
	std::ifstream fstream(path.c_str());
	
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
	weatherLoader.load(path);
}

void GameData::loadHandling(const std::string& path)
{
	GenericDATLoader l;

	l.loadHandling(path, vehicleInfo);
}

SCMFile *GameData::loadSCM(const std::string &path)
{
	std::ifstream f(datpath + "/" + path);

	if(! f.is_open() ) return nullptr;

	f.seekg(0, std::ios_base::end);
	unsigned int size = f.tellg();
	f.seekg(0);

	char* buff = new char[size];
	f.read(buff, size);
	SCMFile* scm = new SCMFile;
	scm->loadFile(buff, size);
	delete[] buff;

	return scm;
}

void GameData::loadGXT(const std::string &name)
{
	auto d = openFile(name);

	LoaderGXT loader;

	loader.load( texts, d );
}

void GameData::loadWaterpro(const std::string& path)
{
	std::ifstream ifstr(path.c_str());
	
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

	auto j = new LoadTextureArchiveJob(this->engine->_work, this, name);

	if( async ) {
		this->engine->_work->queueJob( j );
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

	models[realname] = new ModelHandle(realname);

	auto j = new LoadModelJob(this->engine->_work, this, name,
							  [&, realname]( Model* model ) {
								  models[realname]->model = model;
							  }
						  );


	if( async ) {
		this->engine->_work->queueJob( j );
	}
	else {
		j->work();
		j->complete();
		delete j;
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

void GameData::loadWeaponDAT(const std::string &name)
{
	GenericDATLoader l;

	l.loadWeapons(name, weaponData);
}

bool GameData::loadAudioStream(const std::string &name)
{
	auto fname = findPathRealCase(datpath + "/audio/", name);
	
	if ( engine->cutsceneAudio )
	{
		delete engine->cutsceneAudio;
		engine->cutsceneAudio = nullptr;
	}
	
	bool result = false;
	if ( name.find(".mp3") != name.npos )
	{
		auto stream = new MADStream;
		engine->cutsceneAudio = stream;
		result = stream->openFromFile(fname);
	}
	else
	{
		auto stream = new sf::Music;
		engine->cutsceneAudio = stream;
		result = stream->openFromFile(fname);
	}
	
	return result;
}

bool GameData::loadAudioClip(const std::string& name)
{
	auto fname = findPathRealCase(datpath + "/audio/", name);
	
	if ( engine->missionAudio )
	{
		delete engine->missionAudio;
		engine->missionAudio = nullptr;
	}
	
	if ( name.find(".mp3") != name.npos )
	{
		engine->logger.error("Data", "MP3 Audio unsupported outside cutscenes");
		return false;
	}
	
	engine->missionAudio = new sf::SoundBuffer;
	
	bool r = engine->missionAudio->loadFromFile(fname);
	
	if (! r )
	{
		engine->logger.error("Data", "Error loading audio clip " + fname);
		delete engine->missionAudio;
		engine->missionAudio = nullptr;
	}
	
	return r;
}

void GameData::loadSplash(const std::string &name)
{
	std::string lower(name);
	std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

	loadTXD(lower + ".txd", false);

	engine->state.currentSplash = lower;
}

FileHandle GameData::openFile(const std::string &name)
{
	auto file = index.openFile(name);
	if( file == nullptr )
	{
		engine->logger.error("Data", "Unable to open file: " + name);
	}
	return file;
}

TextureAtlas* GameData::getAtlas(size_t i)
{
	if( i < atlases.size() ) {
		return atlases[i];
	}
	if( i == atlases.size() && (i == 0 || atlases[i-1]->getTextureCount() > 0) ) {
		GLint max_texture_size;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);
		atlases.push_back(new TextureAtlas(max_texture_size/2, max_texture_size/2));
		return atlases[i];
	}
	return nullptr;
}

int GameData::getWaterIndexAt(const glm::vec3 &ws) const
{
	auto wX = (int) ((ws.x + WATER_WORLD_SIZE/2.f) / (WATER_WORLD_SIZE/WATER_HQ_DATA_SIZE));
	auto wY = (int) ((ws.y + WATER_WORLD_SIZE/2.f) / (WATER_WORLD_SIZE/WATER_HQ_DATA_SIZE));

	if( wX >= 0 && wX < WATER_HQ_DATA_SIZE && wY >= 0 && wY < WATER_HQ_DATA_SIZE ) {
		int i = (wX*WATER_HQ_DATA_SIZE) + wY;
		return engine->gameData.realWater[i];
	}
	return 0;
}

float GameData::getWaveHeightAt(const glm::vec3 &ws) const
{
	return (1+sin(engine->gameTime + (ws.x + ws.y) * WATER_SCALE)) * WATER_HEIGHT;
}
