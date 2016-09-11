#include <data/Model.hpp>
#include <data/ModelData.hpp>
#include <data/WeaponData.hpp>
#include <engine/GameData.hpp>
#include <engine/GameState.hpp>
#include <engine/GameWorld.hpp>
#include <loaders/LoaderCOL.hpp>
#include <loaders/LoaderDFF.hpp>
#include <loaders/LoaderIDE.hpp>
#include <loaders/LoaderIPL.hpp>
#include <script/SCMFile.hpp>

#include <core/Logger.hpp>
#include <loaders/BackgroundLoader.hpp>
#include <loaders/GenericDATLoader.hpp>
#include <loaders/LoaderGXT.hpp>
#include <platform/FileIndex.hpp>

#include <algorithm>
#include <boost/algorithm/string/predicate.hpp>
#include <fstream>
#include <iostream>
#include <sstream>

GameData::GameData(Logger* log, WorkContext* work, const std::string& path)
    : datpath(path), logger(log), workContext(work), engine(nullptr) {
}

GameData::~GameData() {
    /// @todo don't leak models
}

void GameData::load() {
    index.indexGameDirectory(datpath);
    index.indexTree(datpath);

    loadIMG("models/gta3.img");
    /// @todo cuts.img files should be loaded differently to gta3.img
    loadIMG("anim/cuts.img");

    loadLevelFile("data/default.dat");
    loadLevelFile("data/gta3.dat");

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

void GameData::loadLevelFile(const std::string& path) {
    auto datpath = index.findFilePath(path);
    std::ifstream datfile(datpath.string());

    if (!datfile.is_open()) {
        logger->error("Data", "Failed to open game file " + path);
        return;
    }

    for (std::string line, cmd; std::getline(datfile, line);) {
        if (line.size() == 0 || line[0] == '#') continue;
#ifndef RW_WINDOWS
        line.erase(line.size() - 1);
#endif

        size_t space = line.find_first_of(' ');
        if (space != line.npos) {
            cmd = line.substr(0, space);
            if (cmd == "IDE") {
                auto path = line.substr(space + 1);
                loadIDE(path);
            } else if (cmd == "SPLASH") {
                splash = line.substr(space + 1);
            } else if (cmd == "COLFILE") {
                int zone = atoi(line.substr(space + 1, 1).c_str());
                auto path = line.substr(space + 3);
                loadCOL(zone, path);
            } else if (cmd == "IPL") {
                auto path = line.substr(space + 1);
                loadIPL(path);
            } else if (cmd == "TEXDICTION") {
                auto path = line.substr(space + 1);
                /// @todo improve TXD handling
                auto name = index.findFilePath(path).filename().string();
                std::transform(name.begin(), name.end(), name.begin(),
                               ::tolower);
                loadTXD(name);
            } else if (cmd == "MODELFILE") {
                auto path = line.substr(space + 1);
                loadModelFile(path);
            }
        }
    }
}

void GameData::loadIDE(const std::string& path) {
    auto systempath = index.findFilePath(path).string();
    LoaderIDE idel;

    if (idel.load(systempath)) {
        std::move(idel.objects.begin(), idel.objects.end(),
                  std::inserter(modelinfo, modelinfo.end()));
    } else {
        logger->error("Data", "Failed to load IDE " + path);
    }
}

uint16_t GameData::findModelObject(const std::string model) {
    auto defit = std::find_if(modelinfo.begin(), modelinfo.end(),
                              [&](const decltype(modelinfo)::value_type& d) {
                                  return boost::iequals(d.second->name, model);
                              });
    if (defit != modelinfo.end()) return defit->first;
    return -1;
}

void GameData::loadCOL(const size_t zone, const std::string& name) {
    RW_UNUSED(zone);

    LoaderCOL col;

    auto systempath = index.findFilePath(name).string();

    if (col.load(systempath)) {
        for (size_t i = 0; i < col.instances.size(); ++i) {
            collisions[col.instances[i]->name] = std::move(col.instances[i]);
        }
    }
}

void GameData::loadIMG(const std::string& name) {
    auto syspath = index.findFilePath(name).string();
    index.indexArchive(syspath);
}

void GameData::loadIPL(const std::string& path) {
    auto systempath = index.findFilePath(path).string();
    iplLocations.insert({path, systempath});
}

bool GameData::loadZone(const std::string& path) {
    LoaderIPL ipll;

    if (ipll.load(path)) {
        if (ipll.zones.size() > 0) {
            for (auto& z : ipll.zones) {
                zones.insert({z.name, z});
            }
            logger->info("Data", "Loaded " + std::to_string(ipll.zones.size()) +
                                     " zones from " + path);
            return true;
        }
    } else {
        logger->error("Data", "Failed to load zones from " + path);
    }

    return false;
}

enum ColSection {
    Unknown,
    COL,
    CAR,
    CAR3,  ///> Used in GTASA, contains extra specular color
    CAR4   ///> Used in GTASA, contains quadruple colors
};

void GameData::loadCarcols(const std::string& path) {
    auto syspath = index.findFilePath(path);
    std::ifstream fstream(syspath.string());

    std::string line;
    ColSection currentSection = Unknown;
    while (std::getline(fstream, line)) {
        if (line.substr(0, 1) == "#") {  // Comment
            continue;
        } else if (currentSection == Unknown) {
            if (line.substr(0, 3) == "col") {
                currentSection = COL;
            } else if (line.substr(0, 3) == "car") {
                currentSection = CAR;
            }
        } else if (line.substr(0, 3) == "end") {
            currentSection = Unknown;
        } else {
            if (currentSection == COL) {
                std::string r, g, b;
                std::stringstream ss(line);

                if (std::getline(ss, r, ',') && std::getline(ss, g, ',') &&
                    std::getline(ss, b)) {
                    vehicleColours.push_back(glm::u8vec3(
                        atoi(r.c_str()), atoi(g.c_str()), atoi(b.c_str())));
                }
            } else if (currentSection == CAR) {
                std::string vehicle, p, s;
                std::stringstream ss(line);

                std::getline(ss, vehicle, ',');
                std::vector<std::pair<size_t, size_t>> colours;

                while (std::getline(ss, p, ',') && std::getline(ss, s, ',')) {
                    colours.push_back({atoi(p.c_str()), atoi(s.c_str())});
                }

                vehiclePalettes.insert({vehicle, colours});
            }
        }
    }
}

void GameData::loadWeather(const std::string& path) {
    auto syspath = index.findFilePath(path).string();
    weatherLoader.load(syspath);
}

void GameData::loadHandling(const std::string& path) {
    GenericDATLoader l;
    auto syspath = index.findFilePath(path).string();

    l.loadHandling(syspath, vehicleInfo);
}

SCMFile* GameData::loadSCM(const std::string& path) {
    auto scm_h = index.openFilePath(path);
    SCMFile* scm = new SCMFile;
    scm->loadFile(scm_h->data, scm_h->length);
    scm_h.reset();
    return scm;
}

void GameData::loadGXT(const std::string& name) {
    auto file = index.openFilePath(name);

    LoaderGXT loader;

    loader.load(texts, file);
}

void GameData::loadWaterpro(const std::string& path) {
    auto syspath = index.findFilePath(path);
    std::ifstream ifstr(syspath.string(), std::ios_base::binary);

    if (ifstr.is_open()) {
        uint32_t numlevels;
        ifstr.read(reinterpret_cast<char*>(&numlevels), sizeof(uint32_t));
        ifstr.read(reinterpret_cast<char*>(&waterHeights), sizeof(float) * 48);

        ifstr.seekg(0x03C4);
        ifstr.read(reinterpret_cast<char*>(&visibleWater),
                   sizeof(char) * 64 * 64);
        ifstr.read(reinterpret_cast<char*>(&realWater),
                   sizeof(char) * 128 * 128);
    }
}

void GameData::loadWater(const std::string& path) {
    std::ifstream ifstr(path.c_str());

    std::string line;
    while (std::getline(ifstr, line)) {
        if (line[0] == ';') {
            continue;
        }

        std::stringstream ss(line);

        std::string a, b, c, d, e;
        float fa, fb, fc, fd, fe;

        if (std::getline(ss, a, ',') && std::getline(ss, b, ',') &&
            std::getline(ss, c, ',') && std::getline(ss, d, ',') &&
            std::getline(ss, e, ',')) {
            fa = atof(a.c_str());
            fb = atof(b.c_str());
            fc = atof(c.c_str());
            fd = atof(d.c_str());
            fe = atof(e.c_str());
            waterBlocks.push_back({fa, fb, fc, fd, fe});
        }
    }
}

void GameData::loadTXD(const std::string& name, bool async) {
    if (loadedFiles.find(name) != loadedFiles.end()) {
        return;
    }

    loadedFiles[name] = true;

    auto j = new LoadTextureArchiveJob(workContext, &index, textures, name);

    if (async) {
        workContext->queueJob(j);
    } else {
        j->work();
        j->complete();
        delete j;
    }
}

void GameData::getNameAndLod(std::string& name, int& lod) {
    auto lodpos = name.rfind("_l");
    if (lodpos != std::string::npos) {
        lod = std::atoi(name.substr(lodpos + 1).c_str());
        name = name.substr(0, lodpos);
    }
}

Model* GameData::loadClump(const std::string& name) {
    auto file = index.openFile(name);
    if (!file) {
        logger->error("Data", "Failed to load model " + name);
        return nullptr;
    }
    LoaderDFF l;
    auto m = l.loadFromMemory(file);
    if (!m) {
        logger->error("Data", "Error loading model file " + name);
        return nullptr;
    }
    return m;
}

void GameData::loadModelFile(const std::string& name) {
    auto file = index.openFilePath(name);
    if (!file) {
        logger->log("Data", Logger::Error, "Failed to load model file " + name);
        return;
    }
    LoaderDFF l;
    auto m = l.loadFromMemory(file);
    if (!m) {
        logger->log("Data", Logger::Error, "Error loading model file " + name);
        return;
    }

    // Associate the frames with models.
    for (auto& frame : m->frames) {
        /// @todo this is useful elsewhere, please move elsewhere
        std::string name = frame->getName();
        int lod = 0;
        getNameAndLod(name, lod);
        for (auto& model : modelinfo) {
            auto info = model.second.get();
            if (info->type() != ModelDataType::SimpleInfo) {
                continue;
            }
            if (boost::iequals(info->name, name)) {
                auto simple = static_cast<SimpleModelInfo*>(info);
                simple->setAtomic(m, lod, frame);
            }
        }
    }
}

void GameData::loadModel(ModelID model) {
    auto info = modelinfo[model].get();
    /// @todo replace openFile with API for loading from CDIMAGE archives
    auto name = info->name;

    // Re-direct special models
    switch (info->type()) {
        case ModelDataType::ClumpInfo:
            // Re-direct the hier objects to the special object ids
            name = engine->state->specialModels[info->id()];
            /// @todo remove this from here
            loadTXD(name + ".txd");
            break;
        case ModelDataType::PedInfo:
            static const std::string specialPrefix("special");
            if (!name.compare(0, specialPrefix.size(), specialPrefix)) {
                auto sid = name.substr(specialPrefix.size());
                unsigned short specialID = std::atoi(sid.c_str());
                name = engine->state->specialCharacters[specialID];
                /// @todo remove this from here
                loadTXD(name + ".txd");
                break;
            }
        default:
            break;
    }

    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
    auto file = index.openFile(name + ".dff");
    if (!file) {
        logger->error("Data", "Failed to load model for " +
                                  std::to_string(model) + " [" + name + "]");
        return;
    }
    LoaderDFF l;
    auto m = l.loadFromMemory(file);
    if (!m) {
        logger->error("Data",
                      "Error loading model file for " + std::to_string(model));
        return;
    }
    /// @todo handle timeinfo models correctly.
    auto isSimple = info->type() == ModelDataType::SimpleInfo;
    if (isSimple) {
        auto simple = static_cast<SimpleModelInfo*>(info);
        // Associate atomics
        for (auto& frame : m->frames) {
            auto name = frame->getName();
            int lod = 0;
            getNameAndLod(name, lod);
            simple->setAtomic(m, lod, frame);
        }
    } else {
        // Associate clumps
        auto clump = static_cast<ClumpModelInfo*>(info);
        clump->setModel(m);
        /// @todo how is LOD handled for clump objects?
    }
}

void GameData::loadIFP(const std::string& name) {
    auto f = index.openFile(name);

    if (f) {
        LoaderIFP loader;
        if (loader.loadFromMemory(f->data)) {
            animations.insert(loader.animations.begin(),
                              loader.animations.end());
        }
    }
}

void GameData::loadDynamicObjects(const std::string& name) {
    GenericDATLoader l;

    l.loadDynamicObjects(name, dynamicObjectData);
}

void GameData::loadWeaponDAT(const std::string& path) {
    GenericDATLoader l;
    auto syspath = index.findFilePath(path).string();

    l.loadWeapons(syspath, weaponData);
}

bool GameData::loadAudioStream(const std::string& name) {
    auto systempath = index.findFilePath("audio/" + name).string();

    if (engine->cutsceneAudio.length() > 0) {
        engine->sound.stopMusic(engine->cutsceneAudio);
    }

    if (engine->sound.loadMusic(name, systempath)) {
        engine->cutsceneAudio = name;
        return true;
    }

    return false;
}

bool GameData::loadAudioClip(const std::string& name,
                             const std::string& fileName) {
    auto systempath = index.findFilePath("audio/" + fileName).string();

    if (systempath.find(".mp3") != std::string::npos) {
        logger->error("Data", "MP3 Audio unsupported outside cutscenes");
        return false;
    }

    bool loaded = engine->sound.loadSound(name, systempath);

    if (!loaded) {
        logger->error("Data", "Error loading audio clip " + systempath);
        return false;
    }

    engine->missionAudio = name;

    return true;
}

void GameData::loadSplash(const std::string& name) {
    std::string lower(name);
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    loadTXD(lower + ".txd", false);

    engine->state->currentSplash = lower;
}

int GameData::getWaterIndexAt(const glm::vec3& ws) const {
    auto wX = (int)((ws.x + WATER_WORLD_SIZE / 2.f) /
                    (WATER_WORLD_SIZE / WATER_HQ_DATA_SIZE));
    auto wY = (int)((ws.y + WATER_WORLD_SIZE / 2.f) /
                    (WATER_WORLD_SIZE / WATER_HQ_DATA_SIZE));

    if (wX >= 0 && wX < WATER_HQ_DATA_SIZE && wY >= 0 &&
        wY < WATER_HQ_DATA_SIZE) {
        int i = (wX * WATER_HQ_DATA_SIZE) + wY;
        return engine->data->realWater[i];
    }
    return 0;
}

float GameData::getWaveHeightAt(const glm::vec3& ws) const {
    return (1 + sin(engine->getGameTime() + (ws.x + ws.y) * WATER_SCALE)) *
           WATER_HEIGHT;
}

bool GameData::isValidGameDirectory(const std::string& path) {
    if (path.empty()) {
        return false;
    }

    LoaderIMG i;
    if (!i.load(path + "/models/gta3.img")) {
        return false;
    }

    return true;
}
