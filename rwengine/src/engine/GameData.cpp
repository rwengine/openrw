#include "engine/GameData.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>

#include <boost/algorithm/string/predicate.hpp>

#include <data/Clump.hpp>
#include <rw/casts.hpp>
#include <rw/debug.hpp>
#include <rw/types.hpp>

#include "core/Logger.hpp"
#include "core/Profiler.hpp"
#include "data/CollisionModel.hpp"
#include "engine/GameState.hpp"
#include "engine/GameWorld.hpp"
#include "loaders/LoaderCOL.hpp"
#include "loaders/LoaderIDE.hpp"
#include "loaders/LoaderIFP.hpp"
#include "loaders/LoaderIPL.hpp"
#include "loaders/WeatherLoader.hpp"
#include "platform/FileHandle.hpp"
#include "script/SCMFile.hpp"
#include "loaders/GenericDATLoader.hpp"
#include "loaders/LoaderGXT.hpp"
#include "platform/FileIndex.hpp"

GameData::GameData(Logger* log, const std::filesystem::path& path)
    : datpath(path), logger(log) {
    dffLoader.setTextureLookupCallback(
        [&](const std::string& texture, const std::string&) {
            return findSlotTexture(currenttextureslot, texture);
        });
}

bool GameData::load() {
    if (!isValidGameDirectory()) {
        return false;
    }
    auto loadTimeStart = std::chrono::steady_clock::now();

    index.indexTree(datpath);

    loadIMG("models/gta3.img");
    /// @todo cuts.img files should be loaded differently to gta3.img
    loadIMG("anim/cuts.img");

    textureSlots["particle"] = loadTextureArchive("particle.txd");
    textureSlots["icons"] = loadTextureArchive("icons.txd");
    textureSlots["hud"] = loadTextureArchive("hud.txd");
    textureSlots["fonts"] = loadTextureArchive("fonts.txd");
    textureSlots["generic"] = loadTextureArchive("generic.txd");
    loadToTextureArchive("misc.txd", textureSlots["generic"]);

    loadCarcols("data/carcols.dat");
    loadWeather("data/timecyc.dat");
    loadHandling("data/handling.cfg");
    loadWaterpro("data/waterpro.dat");
    loadWeaponDAT("data/weapon.dat");
    loadPedStats("data/pedstats.dat");
    loadPedRelations("data/ped.dat");

    loadIFP("ped.ifp");

    /// @todo load real data
    pedAnimGroups["player"] = std::make_unique<AnimGroup>(
        AnimGroup::getBuiltInAnimGroup(animations, "player"));

    // Clear existing zones
    gamezones = ZoneDataList{
        {"CITYZON", 0, {-4000.f, -4000.f, -500.f}, {4000.f, 4000.f, 500.f}, 0, 0, 0}};

    loadLevelFile("data/default.dat");
    loadLevelFile("data/gta3.dat");

    // Load ped groups after IDEs so they can resolve
    loadPedGroups("data/pedgrp.dat");
    loadTimeEnd = std::chrono::steady_clock::now();
    loadTime =
        std::chrono::duration_cast<std::chrono::milliseconds>(loadTimeEnd - loadTimeStart);
    logger->info("Game", "Loading everything else took " + std::to_string(loadTime.count()) + " ms");

    return true;
}

void GameData::loadLevelFile(const std::string& path) {
    auto datpath = index.findFilePath(path);
    std::ifstream datfile(datpath.string());

    if (!datfile.is_open()) {
        logger->error("Data", "Failed to open game file " + path);
        return;
    }

    // Reset texture slot
    currenttextureslot = "generic";

    for (std::string line, cmd; std::getline(datfile, line);) {
        if (line.empty() || line[0] == '#') continue;
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
                int zone = lexical_cast<int>(line.substr(space + 1, 1));
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

    for (const auto& model : modelinfo) {
        if (model.second->type() == ModelDataType::SimpleInfo) {
            auto simple = static_cast<SimpleModelInfo*>(model.second.get());
            simple->setupBigBuilding(modelinfo);
        }
    }
}

void GameData::loadIDE(const std::string& path) {
    auto systempath = index.findFilePath(path).string();
    LoaderIDE idel;

    if (idel.load(systempath, pedstats)) {
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

AnimGroup* GameData::getAnimGroup(const std::string& group) {
    auto it = pedAnimGroups.find(group);
    if (it != pedAnimGroups.end()) {
        return it->second.get();
    }

    pedAnimGroups[group] = std::make_unique<AnimGroup>(
        AnimGroup::getBuiltInAnimGroup(animations, group));
    return pedAnimGroups[group].get();
}

void GameData::loadCOL(const size_t zone, const std::string& name) {
    RW_UNUSED(zone);

    LoaderCOL col;

    auto systempath = index.findFilePath(name).string();

    if (col.load(systempath)) {
        // Associate loaded collisions with models
        for (auto& c : col.collisions) {
            // Find by name
            auto id = findModelObject(c->name);
            auto model = modelinfo.find(id);
            if (model == modelinfo.end()) {
                logger->error("Data", "no model for collsion " + c->name);
                continue;
            }
            model->second->setCollisionModel(c);
        }
    }
}

void GameData::loadIMG(const std::string& name) {
    index.indexArchive(name);
}

void GameData::loadIPL(const std::string& path) {
    auto systempath = index.findFilePath(path).string();
    iplLocations.insert({path, systempath});
}

bool GameData::loadZone(const std::string& path) {
    LoaderIPL ipll;

    // Load the zones
    if (!ipll.load(path)) {
        logger->error("Data", "Failed to load zones from " + path);
        return false;
    }

    gamezones.insert(gamezones.end(), ipll.zones.begin(), ipll.zones.end());

    // Build zone hierarchy
    for (ZoneData& zone : gamezones) {
        zone.children_.clear();
        if (&zone == &gamezones.front()) {
            continue;
        }
        gamezones[0].insertZone(zone);
    }

    return true;
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
                    vehicleColours.emplace_back(
                                lexical_cast<int>(r),
                                lexical_cast<int>(g),
                                lexical_cast<int>(b));
                }
            } else if (currentSection == CAR) {
                std::string vehicle, p, s;
                std::stringstream ss(line);

                std::getline(ss, vehicle, ',');
                std::vector<std::pair<size_t, size_t>> colours;

                while (std::getline(ss, p, ',') && std::getline(ss, s, ',')) {
                    colours.emplace_back(lexical_cast<int>(p),
                                         lexical_cast<int>(s));
                }

                vehiclePalettes.insert({vehicle, colours});
            }
        }
    }
}

void GameData::loadWeather(const std::string& path) {
    auto syspath = index.findFilePath(path).string();
    if (!WeatherLoader::load(syspath, weather)) {
        throw std::runtime_error("Loading Weather " + path + " failed");
    }
}

void GameData::loadHandling(const std::string& path) {
    GenericDATLoader l;
    auto syspath = index.findFilePath(path).string();

    l.loadHandling(syspath, vehicleInfos);
}

SCMFile GameData::loadSCM(const std::string& path) {
    auto scm_h = index.openFileRaw(path);
    SCMFile scm{};
    scm.loadFile(scm_h.data.get(), scm_h.length);
    return scm;
}

void GameData::loadGXT(const std::string& name) {
    auto file = index.openFileRaw(name);

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

        if (std::getline(ss, a, ',') && std::getline(ss, b, ',') &&
            std::getline(ss, c, ',') && std::getline(ss, d, ',') &&
            std::getline(ss, e, ',')) {

            waterBlocks.emplace_back(
                lexical_cast<float>(a),
                lexical_cast<float>(b),
                lexical_cast<float>(c),
                lexical_cast<float>(d),
                lexical_cast<float>(e));
        }
    }
}

void GameData::loadTXD(const std::string& name) {
    RW_PROFILE_COUNTER_ADD("loadTXD", 1);
    auto slot = name;
    auto ext = name.find(".txd");
    if (ext != std::string::npos) {
        slot = name.substr(0, ext);
    }

    // Set the current texture slot
    currenttextureslot = slot;

    // Check if this texture slot is loaded already
    auto slotit = textureSlots.find(slot);
    if (slotit != textureSlots.end()) {
        return;
    }

    textureSlots[slot] = loadTextureArchive(name);
}

TextureArchive GameData::loadTextureArchive(const std::string& name) {
    RW_PROFILE_COUNTER_ADD("loadTextureArchive", 1);
    /// @todo refactor loadTXD to use correct file locations
    auto file = index.openFile(name);
    if (!file.data) {
        logger->error("Data", "Failed to open txd: " + name);
        return {};
    }

    TextureArchive textures;

    TextureLoader l;
    if (!l.loadFromMemory(file, textures)) {
        logger->error("Data", "Error loading txd: " + name);
        return {};
    }

    return textures;
}

void GameData::loadToTextureArchive(const std::string& name,
                                  TextureArchive& archive) {
    RW_PROFILE_COUNTER_ADD("loadTextureArchive", 1);
    /// @todo refactor loadTXD to use correct file locations
    auto file = index.openFile(name);
    if (!file.data) {
        logger->error("Data", "Failed to open txd: " + name);
    }

    TextureLoader l;
    if (!l.loadFromMemory(file, archive)) {
        logger->error("Data", "Error loading txd: " + name);
    }
}

void GameData::getNameAndLod(std::string& name, int& lod) {
    auto lodpos = name.rfind("_l");
    if (lodpos != std::string::npos) {
        lod = lexical_cast<int>(name.substr(lodpos + 2));
        name = name.substr(0, lodpos);
    }
}

ClumpPtr GameData::loadClump(const std::string& name) {
    auto file = index.openFile(name);
    if (!file.data) {
        logger->error("Data", "Failed to load model " + name);
        return nullptr;
    }
    auto m = dffLoader.loadFromMemory(file);
    if (!m) {
        logger->error("Data", "Error loading model file " + name);
        return nullptr;
    }
    return m;
}

ClumpPtr GameData::loadClump(const std::string& name, const std::string& textureSlot) {
    std::string currentSlot = currenttextureslot;
    if (!textureSlot.empty())
        currenttextureslot = textureSlot;
    ClumpPtr result = loadClump(name);
    currenttextureslot = currentSlot;
    return result;
}

void GameData::loadModelFile(const std::string& name) {
    auto file = index.openFileRaw(name);
    if (!file.data) {
        logger->log("Data", Logger::Error, "Failed to load model file " + name);
        return;
    }
    auto m = dffLoader.loadFromMemory(file);
    if (!m) {
        logger->log("Data", Logger::Error, "Error loading model file " + name);
        return;
    }

    // Associate the frames with models.
    for (const auto& atomic : m->getAtomics()) {
        /// @todo this is useful elsewhere, please move elsewhere
        std::string name = atomic->getFrame()->getName();
        int lod = 0;
        getNameAndLod(name, lod);
        for (auto& model : modelinfo) {
            auto info = model.second.get();
            if (info->type() != ModelDataType::SimpleInfo) {
                continue;
            }
            if (boost::iequals(info->name, name)) {
                auto simple = static_cast<SimpleModelInfo*>(info);
                simple->setAtomic(m, lod, atomic);
                auto identity = std::make_shared<ModelFrame>();
                atomic->setFrame(identity);
            }
        }
    }
}

bool GameData::loadModel(ModelID model) {
    auto info = modelinfo[model].get();
    /// @todo replace openFile with API for loading from CDIMAGE archives
    auto name = info->name;
    auto slotname = info->textureslot;

    // Re-direct special models
    switch (info->type()) {
        case ModelDataType::ClumpInfo:
            // Re-direct the hier objects to the special object ids
            name = engine->state->specialModels[info->id()];
            slotname = name;
            break;
        case ModelDataType::PedInfo: {
            static const std::string specialPrefix("special");
            if (!name.compare(0, specialPrefix.size(), specialPrefix)) {
                auto sid = name.substr(specialPrefix.size());
                unsigned short specialID = lexical_cast<int>(sid);
                name = engine->state->specialCharacters[specialID];
                slotname = name;
                break;
            }
        }
        default:
            break;
    }

    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
    std::transform(slotname.begin(), slotname.end(), slotname.begin(),
                   ::tolower);

    /// @todo remove this from here
    loadTXD(slotname + ".txd");

    auto file = index.openFile(name + ".dff");
    if (!file.data) {
        logger->error("Data", "Failed to load model for " +
                                  std::to_string(model) + " [" + name + "]");
        return false;
    }
    auto m = dffLoader.loadFromMemory(file);
    if (!m) {
        logger->error("Data",
                      "Error loading model file for " + std::to_string(model));
        return false;
    }
    /// @todo handle timeinfo models correctly.
    auto isSimple = info->type() == ModelDataType::SimpleInfo;
    if (isSimple) {
        auto simple = static_cast<SimpleModelInfo*>(info);
        // Associate atomics
        for (auto& atomic : m->getAtomics()) {
            auto name = atomic->getFrame()->getName();
            int lod = 0;
            getNameAndLod(name, lod);
            simple->setAtomic(m, lod, atomic);
            auto identity = std::make_shared<ModelFrame>();
            atomic->setFrame(identity);
        }
    } else {
        // Associate clumps
        auto clump = static_cast<ClumpModelInfo*>(info);
        clump->setModel(m);
        /// @todo how is LOD handled for clump objects?
    }

    return true;
}

void GameData::loadIFP(const std::string& name, bool cutsceneAnimation) {
    auto f = index.openFile(name);

    if (f.data) {
        if (LoaderIFP loader{}; loader.loadFromMemory(f.data.get())) {
            auto& dest = cutsceneAnimation ? animationsCutscene : animations;
            dest.insert(loader.animations.begin(), loader.animations.end());
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

void GameData::loadPedStats(const std::string& path) {
    auto syspath = index.findFilePath(path).string();
    std::ifstream fs(syspath.c_str());
    if (!fs.is_open()) {
        throw std::runtime_error("Failed to open " + path);
    }

    std::string line;
    for (int i = 0; std::getline(fs, line);) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        // The name should be ignored, but we will use it anyway
        PedStats stats;
        stats.id_ = i++;
        std::stringstream ss(line);
        ss >> stats.name_;
        ss >> stats.fleedistance_;
        ss >> stats.rotaterate_;
        ss >> stats.fear_;
        ss >> stats.temper_;
        ss >> stats.lawful_;
        ss >> stats.sexy_;
        ss >> stats.attackstrength_;
        ss >> stats.defendweakness_;
        ss >> stats.flags_;

        pedstats.push_back(stats);
    }
}

void GameData::loadPedRelations(const std::string& path) {
    auto syspath = index.findFilePath(path).string();
    std::ifstream fs(syspath.c_str());
    if (!fs.is_open()) {
        throw std::runtime_error("Failed to open " + path);
    }

    std::string line;
    for (int index = 0; std::getline(fs, line);) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        std::stringstream ss(line);
        if (isspace(line[0])) {
            // Add this flags to the last index
            ss >> line;
            if (line == "Avoid") {
                while (!ss.eof()) {
                    ss >> line;
                    auto flag = PedRelationship::threatFromName(line);
                    pedrels[index].avoidflags_ |= flag;
                }
            }
            if (line == "Threat") {
                while (!ss.eof()) {
                    ss >> line;
                    auto flag = PedRelationship::threatFromName(line);
                    pedrels[index].threatflags_ |= flag;
                }
            }
        } else {
            ss >> line;
            index = PedModelInfo::findPedType(line);
            PedRelationship& shp = pedrels[index];
            shp.id_ = PedRelationship::threatFromName(line);
            ss >> shp.a_;
            ss >> shp.b_;
            ss >> shp.c_;
            ss >> shp.d_;
            ss >> shp.e_;
        }
    }
}

void GameData::loadPedGroups(const std::string& path) {
    auto syspath = index.findFilePath(path).string();
    std::ifstream fs(syspath.c_str());
    if (!fs.is_open()) {
        throw std::runtime_error("Failed to open " + path);
    }

    std::string line;
    while (std::getline(fs, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        std::stringstream ss(line);
        PedGroup group;
        while (ss >> line) {
            if (line.empty() || line[0] == '#') {
                break;
            }
            if (line.back() == ',') {
                line.resize(line.size() - 1);
            }
            auto model = findModelObject(line);
            if (int16_t(model) == -1) {
                logger->error("Data", "Invalid model in ped group " + line);
                continue;
            }
            group.push_back(model);
        }
        if (!group.empty()) {
            pedgroups.emplace_back(std::move(group));
        }
    }
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

    textureSlots[lower + ".txd"] = loadTextureArchive(lower + ".txd");

    engine->state->currentSplash = lower;
}

TextureData* GameData::findSlotTexture(const std::string &slot, const std::string &texture) const {
    auto slotIt = textureSlots.find(slot);
    if (slotIt == textureSlots.end()) {
        return nullptr;
    }
    auto textureIt = slotIt->second.find(texture);
    if (textureIt == slotIt->second.end()) {
        return nullptr;
    }
    return textureIt->second.get();
}

ZoneData *GameData::findZone(const std::string &name) {
    auto it =
            std::find_if(gamezones.begin(), gamezones.end(),
                         [&](const ZoneData& a) { return a.name == name; });
    return it != gamezones.end() ? &(*it) : nullptr;
}

ZoneData *GameData::findZoneAt(const glm::vec3 &pos) {
    RW_CHECK(!gamezones.empty(), "No game zones loaded");
    ZoneData* zone = gamezones[0].findLeafAtPoint(pos);
    return zone;
}

int GameData::getWaterIndexAt(const glm::vec3& ws) const {
    auto wX = static_cast<int>((ws.x + WATER_WORLD_SIZE / 2.f) /
                               (WATER_WORLD_SIZE / WATER_HQ_DATA_SIZE));
    auto wY = static_cast<int>((ws.y + WATER_WORLD_SIZE / 2.f) /
                               (WATER_WORLD_SIZE / WATER_HQ_DATA_SIZE));

    if (wX >= 0 && wX < WATER_HQ_DATA_SIZE && wY >= 0 &&
            wY < WATER_HQ_DATA_SIZE) {
        int i = (wX * WATER_HQ_DATA_SIZE) + wY;
        return engine->data->realWater[i];
    }
    return 0;
}

float GameData::getWaveHeightAt(const glm::vec3& ws) const {
    return (1 + std::sin(engine->getGameTime() + (ws.x + ws.y) * WATER_SCALE)) *
           WATER_HEIGHT;
}

bool GameData::isValidGameDirectory() const {
    std::error_code ec;
    if (!std::filesystem::is_directory(datpath, ec)) {
        return false;
    }

    return !ec;
}
