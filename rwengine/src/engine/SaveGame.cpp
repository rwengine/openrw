#include "engine/SaveGame.hpp"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdio>

#include <iostream>

#include <rw/filesystem.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <rw/debug.hpp>

#include "data/ZoneData.hpp"
#include "engine/GameData.hpp"
#include "engine/GameState.hpp"
#include "engine/GameWorld.hpp"
#include "engine/SaveGameTypes.hpp"
#include "objects/CharacterObject.hpp"
#include "objects/GameObject.hpp"
#include "objects/InstanceObject.hpp"
#include "objects/VehicleObject.hpp"
#include "script/SCMFile.hpp"
#include "script/ScriptMachine.hpp"
#include "script/ScriptTypes.hpp"


void SaveGame::writeGame(GameState& state, const std::string& file) {
    RW_UNUSED(state);
    RW_UNUSED(file);
    RW_UNIMPLEMENTED("Saving the game is not implemented yet.");
}

template <class T>
bool readBlock(std::FILE* str, T& out) {
    return std::fread(&out, sizeof(out), 1, str) == 1;
}

#define READ_VALUE(var)                                                   \
    if (!readBlock(loadFile, var)) {                                      \
        RW_ERROR(file << ": Failed to load block " #var);                 \
        return false;                                                     \
    }
#define READ_SIZE(var)                                                   \
    if (!readBlock(loadFile, var)) {                                     \
        RW_ERROR(file << ": Failed to load size " #var);                 \
        return false;                                                    \
    }
#define CHECK_SIG(expected)                                               \
    {                                                                     \
        char signature[4];                                                \
        if (fread(signature, sizeof(char), 4, loadFile) != 4) {           \
            RW_ERROR("Failed to read signature");                         \
            return false;                                                 \
        }                                                                 \
        if (strncmp(signature, expected, 3) != 0) {                       \
            RW_ERROR("Signature " expected " incorrect");                 \
            return false;                                                 \
        }                                                                 \
    }
#define BLOCK_HEADER(sizevar)             \
    fseek(loadFile, nextBlock, SEEK_SET); \
    READ_SIZE(sizevar)                    \
    nextBlock += sizeof(sizevar) + sizevar;

bool SaveGame::loadGame(GameState& state, const std::string& file) {
    std::FILE* loadFile = std::fopen(file.c_str(), "rb");
    if (loadFile == nullptr) {
        RW_ERROR("Failed to open save file");
        return false;
    }

    BlockSize nextBlock = 0;

    // BLOCK 0
    BlockDword blockSize;
    BLOCK_HEADER(blockSize);

    static_assert(sizeof(BasicState) == 0xBC,
                  "BasicState is not the right size");
    READ_VALUE(state.basic)

    BlockDword scriptBlockSize;

    READ_SIZE(scriptBlockSize)
    CHECK_SIG("SCR")
    READ_SIZE(scriptBlockSize)

    BlockDword scriptVarCount;
    READ_SIZE(scriptVarCount)
    RW_ASSERT(scriptVarCount == state.script->getFile().getGlobalsSize());

    if (fread(state.script->getGlobals(), sizeof(SCMByte), scriptVarCount,
              loadFile) != scriptVarCount) {
        RW_ERROR("Failed to read script memory");
        return false;
    }

    BlockDword scriptDataBlockSize;
    READ_SIZE(scriptDataBlockSize);
    if (scriptDataBlockSize != 0x03C8) {
        return false;
    }

    Block0ScriptData scriptData;
    READ_VALUE(scriptData);

    BlockDword numScripts;
    READ_SIZE(numScripts)
    std::vector<Block0RunningScript> scripts(numScripts);
    for (size_t i = 0; i < numScripts; ++i) {
        READ_VALUE(scripts[i]);
    }

    // BLOCK 1
    BlockDword playerBlockSize;
    BLOCK_HEADER(playerBlockSize);
    BlockDword playerInfoSize;
    READ_SIZE(playerInfoSize)
    BlockDword playerCount;
    READ_SIZE(playerCount)

    std::vector<Block1PlayerPed> players(playerCount);
    for (unsigned int p = 0; p < playerCount; ++p) {
        Block1PlayerPed& ped = players[p];
        READ_VALUE(ped.unknown0)
        READ_VALUE(ped.unknown1)
        READ_VALUE(ped.reference)
        READ_VALUE(ped.info)
        READ_VALUE(ped.maxWantedLevel)
        READ_VALUE(ped.maxChaosLevel)
        READ_VALUE(ped.modelName)
        READ_VALUE(ped.align)

#ifdef RW_DEBUG
        std::cout << "Player Health: " << ped.info.health << " ("
                  << ped.info.armour << ")" << std::endl;
        std::cout << "Player model: " << ped.modelName << std::endl;
        for (const auto &wep : players[p].info.weapons) {
            std::cout << "ID " << wep.weaponId << " " << wep.inClip << " "
                      << wep.totalBullets << std::endl;
        }
#endif
    }

    // BLOCK 2
    BlockDword garageBlockSize;
    BLOCK_HEADER(garageBlockSize);
    BlockDword garageDataSize;
    READ_SIZE(garageDataSize)

    Block2GarageData garageData;
    READ_VALUE(garageData.garageCount)
    READ_VALUE(garageData.freeBombs)
    READ_VALUE(garageData.freeResprays)
    READ_VALUE(garageData.unknown0)
    READ_VALUE(garageData.unknown1)
    READ_VALUE(garageData.unknown2)
    READ_VALUE(garageData.bfImportExportPortland)
    READ_VALUE(garageData.bfImportExportShoreside)
    READ_VALUE(garageData.bfImportExportUnused)
    READ_VALUE(garageData.GA_21lastTime)
    READ_VALUE(garageData.cars)

    std::vector<StructGarage> garages(garageData.garageCount);
    for (size_t i = 0; i < garageData.garageCount; ++i) {
        READ_VALUE(garages[i]);
    }

#ifdef RW_DEBUG
    std::cout << "Garages: " << garageData.garageCount << std::endl;
    std::cout << "Bombs Free: " << garageData.freeBombs << std::endl;
    std::cout << "Sprays Free: " << garageData.freeResprays << std::endl;
    std::cout << "Portland IE: " << garageData.bfImportExportPortland
              << std::endl;
    std::cout << "Shoreside IE: " << garageData.bfImportExportShoreside
              << std::endl;
    std::cout << "GA21 last shown: " << garageData.GA_21lastTime << std::endl;
    for (const auto &car : garageData.cars) {
        if (car.modelId == 0) continue;
        std::cout << " " << car.modelId << " " << uint16_t(car.colorFG) << "/"
                  << uint16_t(car.colorBG) << " " << car.immunities
                  << std::endl;
    }
#endif

    // Block 3
    BlockSize vehicleBlockSize;
    BLOCK_HEADER(vehicleBlockSize);
    BlockSize vehicleDataSize;
    READ_SIZE(vehicleDataSize)

    BlockDword vehicleCount;
    BlockDword boatCount;
    READ_VALUE(vehicleCount)
    READ_VALUE(boatCount)

    std::vector<Block3Vehicle> vehicles(vehicleCount);
    for (size_t v = 0; v < vehicleCount; ++v) {
        Block3Vehicle& veh = vehicles[v];
        READ_VALUE(veh.unknown1)
        READ_VALUE(veh.modelId)
        READ_VALUE(veh.unknown2)
        READ_VALUE(veh.state)
#ifdef RW_DEBUG
        std::cout << " v " << veh.modelId << " " << veh.state.position.x << " "
                  << veh.state.position.y << " " << veh.state.position.z
                  << std::endl;
#endif
    }
    std::vector<Block3Boat> boats(boatCount);
    for (size_t v = 0; v < boatCount; ++v) {
        Block3Boat& veh = boats[v];
        READ_VALUE(veh.unknown1)
        READ_VALUE(veh.modelId)
        READ_VALUE(veh.unknown2)
        READ_VALUE(veh.state)
#ifdef RW_DEBUG
        std::cout << " b " << veh.modelId << " " << veh.state.position.x << " "
                  << veh.state.position.y << " " << veh.state.position.z
                  << std::endl;
#endif
    }

    // Block 4
    BlockSize objectsBlockSize;
    BLOCK_HEADER(objectsBlockSize)
    BlockDword objectDataSize;
    READ_VALUE(objectDataSize)

    BlockDword objectCount;
    READ_VALUE(objectCount);

    std::vector<Block4Object> objects(objectCount);
    for (size_t o = 0; o < objectCount; ++o) {
        Block4Object& obj = objects[o];
        READ_VALUE(obj.modelId)
        READ_VALUE(obj.reference)
        READ_VALUE(obj.position)
        READ_VALUE(obj.rotation)
        READ_VALUE(obj.unknown1)
        READ_VALUE(obj.unknown2)
        READ_VALUE(obj.unknown3)
        READ_VALUE(obj.unknown4)
        READ_VALUE(obj.unknown5)
        READ_VALUE(obj.unknown6)
        READ_VALUE(obj.unknown7)
        READ_VALUE(obj.unknown8)
        READ_VALUE(obj.unknown9)
        READ_VALUE(obj.unknown10)
    }

    for (size_t o = 0; o < objectCount; ++o) {
        auto& obj = objects[o];
        GameObject* inst =
            state.world->createInstance(obj.modelId, obj.position);
        glm::vec3 right = glm::normalize(
            glm::vec3(obj.rotation[0], obj.rotation[1], obj.rotation[2]));
        glm::vec3 forward = glm::normalize(
            glm::vec3(obj.rotation[3], obj.rotation[4], obj.rotation[5]));
        glm::vec3 down = glm::normalize(
            glm::vec3(obj.rotation[6], obj.rotation[7], obj.rotation[8]));
        glm::mat3 m = glm::mat3(right, forward, -down);
        inst->setRotation(glm::normalize(static_cast<glm::quat>(m)));
    }

#ifdef RW_DEBUG
    std::cout << "Objects " << objectCount << std::endl;
    for (size_t o = 0; o < objectCount; ++o) {
        auto& obj = objects[o];
        glm::vec3 right = glm::normalize(
            glm::vec3(obj.rotation[0], obj.rotation[1], obj.rotation[2]));
        glm::vec3 forward = glm::normalize(
            glm::vec3(obj.rotation[3], obj.rotation[4], obj.rotation[5]));
        glm::vec3 down = glm::normalize(
            glm::vec3(obj.rotation[6], obj.rotation[7], obj.rotation[8]));
        std::cout << "modelId " << obj.modelId << " ";
        std::cout << "position " << obj.position.x << " " << obj.position.y
                  << " " << obj.position.z << " ";
        std::cout << "right " << right.x << " " << right.y << " " << right.z
                  << " ";
        std::cout << "forward " << forward.x << " " << forward.y << " "
                  << forward.z << " ";
        std::cout << "down " << down.x << " " << down.y << " " << down.z
                  << std::endl;
    }
    std::cout << std::endl;
#endif

    // Block 5
    BlockSize pathBlockSize;
    BLOCK_HEADER(pathBlockSize)
    BlockDword pathDataSize;
    READ_VALUE(pathDataSize)

    BlockDword numPaths;
    READ_VALUE(numPaths)
    for (size_t b = 0; b < numPaths; ++b) {
        uint8_t bits;
        READ_VALUE(bits)
    }

    // Block 6
    BlockSize craneBlockSize;
    BLOCK_HEADER(craneBlockSize)
    BlockDword craneDataSize;
    READ_VALUE(craneDataSize)

    Block6Data craneData;
    READ_VALUE(craneData.numCranes)
    READ_VALUE(craneData.militaryCollected)
    for (size_t c = 0; c < craneData.numCranes; ++c) {
        Block6Crane& crane = craneData.cranes[c];
        READ_VALUE(crane)
    }

#ifdef RW_DEBUG
    std::cout << "Cranes: " << craneData.numCranes << std::endl;
    for (size_t c = 0; c < craneData.numCranes; ++c) {
        Block6Crane& crane = craneData.cranes[c];
        std::cout << "pickup " << crane.x1Pickup << " " << crane.y1Pickup << " "
                  << crane.x2Pickup << " " << crane.y2Pickup << std::endl;
        std::cout << "vehicles collected " << uint16_t(crane.vehiclesCollected)
                  << std::endl;
    }
#endif

    // Block 7
    BlockSize pickupBlockSize;
    BLOCK_HEADER(pickupBlockSize)
    BlockDword pickupDataSize;
    READ_VALUE(pickupDataSize)

    Block7Data pickupData;
    READ_VALUE(pickupData);

#ifdef RW_DEBUG
    for (const auto &pickup : pickupData.pickups) {
        if (pickup.type == 0) continue;
        std::cout << " " << uint16_t(pickup.type) << " " << pickup.position.x << " "
                  << pickup.position.y << " " << pickup.position.z << std::endl;
    }
#endif

    // Block 8
    BlockSize payphoneBlockSize;
    BLOCK_HEADER(payphoneBlockSize)
    BlockDword payphoneDataSize;
    READ_VALUE(payphoneDataSize)

    Block8Data payphoneData;
    READ_VALUE(payphoneData);
    std::vector<Block8Payphone> payphones(payphoneData.numPayphones);
    for (auto& payphone : payphones) {
        READ_VALUE(payphone)
    }

#ifdef RW_DEBUG
    std::cout << "Payphones: " << payphoneData.numPayphones << std::endl;
    for (const auto& payphone : payphones) {
        std::cout << " " << uint16_t(payphone.state) << " " << payphone.position.x
                  << " " << payphone.position.y << " " << payphone.position.z
                  << std::endl;
    }
#endif

    // Block 9
    BlockSize restartBlockSize;
    BLOCK_HEADER(restartBlockSize)
    BlockDword restartDataSize;
    READ_VALUE(restartDataSize)
    CHECK_SIG("RST")
    READ_VALUE(restartDataSize)

    Block9Data restartData;
    READ_VALUE(restartData);

#ifdef RW_DEBUG
    std::cout << "Hospitals: " << restartData.numHospitals
              << " police: " << restartData.numPolice << std::endl;
    for (int s = 0; s < restartData.numHospitals; ++s) {
        Block9Restart& p = restartData.hospitalRestarts[s];
        std::cout << " H " << p.position.x << " " << p.position.y << " "
                  << p.position.z << std::endl;
    }
    for (int s = 0; s < restartData.numPolice; ++s) {
        Block9Restart& p = restartData.policeRestarts[s];
        std::cout << " P " << p.position.x << " " << p.position.y << " "
                  << p.position.z << std::endl;
    }
#endif

    // Block 10
    BlockSize radarBlockSize;
    BLOCK_HEADER(radarBlockSize)
    BlockDword radarDataSize;
    READ_VALUE(radarDataSize)
    CHECK_SIG("RDR")
    READ_VALUE(radarDataSize)

    Block10Data radarData;
    READ_VALUE(radarData);

#ifdef RW_DEBUG
    for (const auto &blip : radarData.blips) {
        if (blip.type == 0) continue;
        std::cout << " " << blip.position.x << " " << blip.position.y << " "
                  << blip.position.z << std::endl;
    }
#endif

    // Block 11
    BlockSize zoneBlockSize;
    BLOCK_HEADER(zoneBlockSize)
    BlockDword zoneDataSize;
    READ_VALUE(zoneDataSize)
    CHECK_SIG("ZNS")
    READ_VALUE(zoneDataSize)

    Block11Data zoneData;
    READ_VALUE(zoneData.currentZone);
    READ_VALUE(zoneData.currentLevel);
    READ_VALUE(zoneData.findIndex);
    READ_VALUE(zoneData.align);
    for (auto &zone : zoneData.navZones) {
        READ_VALUE(zone.name);
        READ_VALUE(zone.coordA);
        READ_VALUE(zone.coordB);
        READ_VALUE(zone.type);
        READ_VALUE(zone.level);
        READ_VALUE(zone.dayZoneInfo);
        READ_VALUE(zone.nightZoneInfo);
        READ_VALUE(zone.childZone);
        READ_VALUE(zone.parentZone);
        READ_VALUE(zone.siblingZone);
    }
    for (auto &info : zoneData.dayNightInfo) {
        READ_VALUE(info.density)
        READ_VALUE(info.unknown1)
        READ_VALUE(info.peddensity)
        READ_VALUE(info.copdensity)
        READ_VALUE(info.gangpeddensity)
        READ_VALUE(info.pedgroup);
    }
    READ_VALUE(zoneData.numNavZones);
    READ_VALUE(zoneData.numZoneInfos);
    for (auto &zone : zoneData.mapZones) {
        READ_VALUE(zone.name);
        READ_VALUE(zone.coordA);
        READ_VALUE(zone.coordB);
        READ_VALUE(zone.type);
        READ_VALUE(zone.level);
        READ_VALUE(zone.dayZoneInfo);
        READ_VALUE(zone.nightZoneInfo);
        READ_VALUE(zone.childZone);
        READ_VALUE(zone.parentZone);
        READ_VALUE(zone.siblingZone);
    }
    for (auto &audioZone : zoneData.audioZones) {
        READ_VALUE(audioZone);
    }
    READ_VALUE(zoneData.numMapZones);
    READ_VALUE(zoneData.numAudioZones);

#ifdef RW_DEBUG
    std::cout << "zones: " << zoneData.numNavZones << " "
              << zoneData.numZoneInfos << " " << zoneData.numMapZones << " "
              << zoneData.numAudioZones << std::endl;
    for (int z = 0; z < zoneData.numNavZones; ++z) {
        Block11Zone& zone = zoneData.navZones[z];
        std::cout << " " << zone.name << std::endl;
        auto& dayinfo = zoneData.dayNightInfo[zone.dayZoneInfo];
        std::cout << "  DAY " << dayinfo.density << " " << dayinfo.peddensity
                  << " " << dayinfo.copdensity << " "
                  << " [";
        for (BlockDword gang : dayinfo.gangpeddensity) {
            std::cout << " " << gang;
        }
        std::cout << "] " << dayinfo.pedgroup << std::endl;
        for (BlockDword dw : dayinfo.unknown1) {
            std::cout << " " << dw;
        }
        std::cout << std::endl;

        auto& nightinfo = zoneData.dayNightInfo[zone.nightZoneInfo];
        std::cout << "  NIGHT " << nightinfo.density << " "
                  << nightinfo.peddensity << " " << nightinfo.copdensity << " "
                  << " [";
        for (BlockDword gang : nightinfo.gangpeddensity) {
            std::cout << " " << gang;
        }
        std::cout << "] " << nightinfo.pedgroup << std::endl;
        for (BlockDword dw : nightinfo.unknown1) {
            std::cout << " " << dw;
        }
        std::cout << std::endl;
    }
    for (int z = 0; z < zoneData.numMapZones; ++z) {
        Block11Zone& zone = zoneData.mapZones[z];
        std::cout << " " << zone.name << std::endl;
    }
#endif

    // Clear existing zone data
    auto& gamezones = state.world->data->gamezones;
    gamezones.clear();
    for (int z = 0; z < zoneData.numNavZones; ++z) {
        Block11Zone& zone = zoneData.navZones[z];
        Block11ZoneInfo& day = zoneData.dayNightInfo[zone.dayZoneInfo];
        Block11ZoneInfo& night = zoneData.dayNightInfo[zone.nightZoneInfo];
        // @toodo restore gang density
        gamezones.emplace_back(zone.name, zone.type, zone.coordA, zone.coordB,
                            zone.level, day.pedgroup, night.pedgroup);
    }
    // Re-build zone hierarchy
    for (ZoneData& zone : gamezones) {
        if (&zone == &gamezones[0]) {
            continue;
        }

        gamezones[0].insertZone(zone);
    }

    // Block 12
    BlockSize gangBlockSize;
    BLOCK_HEADER(gangBlockSize)
    BlockDword gangDataSize;
    READ_VALUE(gangDataSize)
    CHECK_SIG("GNG")
    READ_VALUE(gangDataSize)

    Block12Data gangData;
    READ_VALUE(gangData);

#ifdef RW_DEBUG
    for (const auto &gang : gangData.gangs) {
        std::cout << " " << gang.carModelId << " " << gang.weaponPrimary << " "
                  << gang.weaponSecondary << std::endl;
    }
#endif

    // Block 13
    BlockSize carGeneratorBlockSize;
    BLOCK_HEADER(carGeneratorBlockSize)
    BlockDword carGeneratorDataSize;
    READ_VALUE(carGeneratorDataSize)
    CHECK_SIG("CGN")
    READ_VALUE(carGeneratorDataSize)

    Block13Data carGeneratorData;
    READ_VALUE(carGeneratorData);

    std::vector<Block13CarGenerator> carGenerators(
        carGeneratorData.generatorCount);
    for (size_t g = 0; g < carGeneratorData.generatorCount; ++g) {
        READ_VALUE(carGenerators[g])
    }

#ifdef RW_DEBUG
    std::cout << "Car generators: " << carGeneratorData.generatorCount
              << std::endl;
    for (size_t g = 0; g < carGeneratorData.generatorCount; ++g) {
        Block13CarGenerator& gen = carGenerators[g];
        std::cout << " " << gen.modelId << " " << gen.position.x << " "
                  << gen.position.y << " " << gen.position.z << std::endl;
    }
#endif

    // Block 14
    BlockSize particleBlockSize;
    BLOCK_HEADER(particleBlockSize)
    BlockDword particleDataSize;
    READ_VALUE(particleDataSize)

    BlockDword particleCount;
    READ_VALUE(particleCount);
    std::vector<Block14Particle> particles(particleCount);
    for (size_t p = 0; p < particleCount; ++p) {
        READ_VALUE(particles[p])
    }

#ifdef RW_DEBUG
    std::cout << "particles: " << particleCount << std::endl;
#endif

    // Block 15
    BlockSize audioBlockSize;
    BLOCK_HEADER(audioBlockSize)
    BlockDword audioDataSize;
    READ_VALUE(audioDataSize)
    CHECK_SIG("AUD")
    READ_VALUE(audioDataSize)

    BlockDword audioCount;
    READ_VALUE(audioCount)

    std::vector<Block15AudioObject> audioObjects(audioCount);
    for (size_t a = 0; a < audioCount; ++a) {
        READ_VALUE(audioObjects[a])
    }

#ifdef RW_DEBUG
    std::cout << "Audio Objects: " << audioCount << std::endl;
#endif

    // Block 16
    BlockSize playerInfoBlockSize;
    BLOCK_HEADER(playerInfoBlockSize)
    BlockDword playerInfoDataSize;
    READ_VALUE(playerInfoDataSize)
    READ_VALUE(state.playerInfo.money)
    READ_VALUE(state.playerInfo.unknown1)
    READ_VALUE(state.playerInfo.unknown2)
    READ_VALUE(state.playerInfo.unknown3)
    READ_VALUE(state.playerInfo.unknown4)
    READ_VALUE(state.playerInfo.displayedMoney)
    READ_VALUE(state.playerInfo.hiddenPackagesCollected)
    READ_VALUE(state.playerInfo.hiddenPackageCount)
    READ_VALUE(state.playerInfo.neverTired)
    READ_VALUE(state.playerInfo.fastReload)
    READ_VALUE(state.playerInfo.thaneOfLibertyCity)
    READ_VALUE(state.playerInfo.singlePayerHealthcare)
    READ_VALUE(state.playerInfo.unknown5)

#ifdef RW_DEBUG
    std::cout << "Player money: " << state.playerInfo.money << " ("
              << state.playerInfo.displayedMoney << ")" << std::endl;
#endif

    // Block 17
    BlockSize statsBlockSize;
    BLOCK_HEADER(statsBlockSize)
    BlockDword statsDataSize;
    READ_VALUE(statsDataSize)

    READ_VALUE(state.gameStats.playerKills);
    READ_VALUE(state.gameStats.otherKills);
    READ_VALUE(state.gameStats.carsExploded);
    READ_VALUE(state.gameStats.shotsHit);
    READ_VALUE(state.gameStats.pedTypesKilled);
    READ_VALUE(state.gameStats.helicoptersDestroyed);
    READ_VALUE(state.gameStats.playerProgress);
    READ_VALUE(state.gameStats.explosiveKgsUsed);
    READ_VALUE(state.gameStats.bulletsFired);
    READ_VALUE(state.gameStats.bulletsHit);
    READ_VALUE(state.gameStats.carsCrushed);
    READ_VALUE(state.gameStats.headshots);
    READ_VALUE(state.gameStats.timesBusted);
    READ_VALUE(state.gameStats.timesHospital);
    READ_VALUE(state.gameStats.daysPassed);
    READ_VALUE(state.gameStats.mmRainfall);
    READ_VALUE(state.gameStats.insaneJumpMaxDistance);
    READ_VALUE(state.gameStats.insaneJumpMaxHeight);
    READ_VALUE(state.gameStats.insaneJumpMaxFlips);
    READ_VALUE(state.gameStats.insaneJumpMaxRotation);
    READ_VALUE(state.gameStats.bestStunt);
    READ_VALUE(state.gameStats.uniqueStuntsFound);
    READ_VALUE(state.gameStats.uniqueStuntsTotal);
    READ_VALUE(state.gameStats.missionAttempts);
    READ_VALUE(state.gameStats.missionsPassed);
    READ_VALUE(state.gameStats.passengersDroppedOff);
    READ_VALUE(state.gameStats.taxiRevenue);
    READ_VALUE(state.gameStats.portlandPassed);
    READ_VALUE(state.gameStats.stauntonPassed);
    READ_VALUE(state.gameStats.shoresidePassed);
    READ_VALUE(state.gameStats.bestTurismoTime);
    READ_VALUE(state.gameStats.distanceWalked);
    READ_VALUE(state.gameStats.distanceDriven);
    READ_VALUE(state.gameStats.patriotPlaygroundTime);
    READ_VALUE(state.gameStats.aRideInTheParkTime);
    READ_VALUE(state.gameStats.grippedTime);
    READ_VALUE(state.gameStats.multistoryMayhemTime);
    READ_VALUE(state.gameStats.peopleSaved);
    READ_VALUE(state.gameStats.criminalsKilled);
    READ_VALUE(state.gameStats.highestParamedicLevel);
    READ_VALUE(state.gameStats.firesExtinguished);
    READ_VALUE(state.gameStats.longestDodoFlight);
    READ_VALUE(state.gameStats.bombDefusalTime);
    READ_VALUE(state.gameStats.rampagesPassed);
    READ_VALUE(state.gameStats.totalRampages);
    READ_VALUE(state.gameStats.totalMissions);
    READ_VALUE(state.gameStats.fastestTime);
    READ_VALUE(state.gameStats.highestScore);
    READ_VALUE(state.gameStats.peopleKilledSinceCheckpoint);
    READ_VALUE(state.gameStats.peopleKilledSinceLastBustedOrWasted);
    READ_VALUE(state.gameStats.lastMissionGXT);

#ifdef RW_DEBUG
    std::cout << "Player kills: " << state.gameStats.playerKills << std::endl;
    std::cout << "longest flight " << state.gameStats.longestDodoFlight
              << std::endl;
#endif

    // Block 18
    BlockSize streamingBlockSize;
    BLOCK_HEADER(streamingBlockSize);
    BlockDword streamingDataSize;
    READ_VALUE(streamingDataSize);

    Block18Data streamingData;
    READ_VALUE(streamingData);

#if defined(RW_DEBUG) && 0  // No idea what the data in the section means yet
    static const size_t streamSize = sqrt(200 * 8);
    for (int x = 0; x < streamSize; ++x) {
        for (int y = 0; y < streamSize; ++y) {
            size_t coord = (y * streamSize) + x;
            if (streamingData.unknown1[(coord / 8)] & (0x1 << (coord % 8))) {
                std::cout << "\u2588";
            } else {
                std::cout << " ";
            }
        }
        std::cout << std::endl;
    }
#endif

    // Block 19
    BlockSize pedTypeBlockSize;
    BLOCK_HEADER(pedTypeBlockSize);
    BlockDword pedTypeDataSize;
    READ_VALUE(pedTypeDataSize);
    CHECK_SIG("PTP");
    READ_VALUE(pedTypeDataSize);

    Block19Data pedTypeData;
    READ_VALUE(pedTypeData);

#ifdef RW_DEBUG
    for (const auto &type : pedTypeData.types) {
        printf("%08x: %f %f %f %f %f threat %08x avoid %08x\n", type.bitstring_,
               static_cast<double>(type.unknown2),
               static_cast<double>(type.unknown3),
               static_cast<double>(type.unknown4),
               static_cast<double>(type.fleedistance),
               static_cast<double>(type.headingchangerate),
               type.threatflags_,
               type.avoidflags_);
    }
#endif

    // Data Cleanup

    // We keep track of the game time as a float for now
    state.gameTime = state.basic.timeMS / 1000.f;

    state.scriptOnMissionFlag = reinterpret_cast<int32_t*>(
        state.script->getGlobals() +
        static_cast<size_t>(scriptData.onMissionOffset));

    auto& threads = state.script->getThreads();
    for (size_t s = 0; s < numScripts; ++s) {
        state.script->startThread(scripts[s].programCounter);
        SCMThread& thread = threads.back();
        // no baseAddress in III and VC
        strncpy(thread.name, scripts[s].name, sizeof(SCMThread::name) - 1);
        thread.conditionResult = scripts[s].ifFlag;
        thread.conditionCount = scripts[s].ifNumber;
        thread.stackDepth = scripts[s].stackCounter;
        for (int i = 0; i < SCM_STACK_DEPTH; ++i) {
            thread.calls[i] = scripts[s].stack[i];
        }
        /* TODO not hardcode +33 ms */
        thread.wakeCounter = scripts[s].wakeTimer - state.basic.lastTick + 33;
        for (size_t i = 0; i < sizeof(Block0RunningScript::variables); ++i) {
            thread.locals[i] = scripts[s].variables[i];
        }
    }

    if (playerCount > 0) {
        auto& ply = players[0];
        std::cout << ply.reference << std::endl;
        auto player = state.world->createPlayer(players[0].info.position);
        CharacterState& cs = player->getCurrentState();
        cs.health = players[0].info.health;
        cs.armour = players[0].info.armour;
        state.playerObject = player->getGameObjectID();
        state.maxWantedLevel = players[0].maxWantedLevel;
        for (int w = 0; w < kNrOfWeapons; ++w) {
            auto& wep = ply.info.weapons[w];
            cs.weapons[w].weaponId = wep.weaponId;
            cs.weapons[w].bulletsClip = wep.inClip;
            cs.weapons[w].bulletsTotal = wep.totalBullets;
        }
    }

    // @todo restore properly
    for (const auto& payphone : payphones) {
        state.world->createPayphone(glm::vec2(payphone.position));
    }

    // TODO restore garage data
    // http://gtaforums.com/topic/758692-gta-iii-save-file-documentation/
    for (size_t g = 0; g < garageData.garageCount; ++g) {
        auto& garage = garages[g];
        state.world->createGarage(glm::vec3(garage.x1, garage.y1, garage.z1),
                                  glm::vec3(garage.x2, garage.y2, garage.z2),
                                  static_cast<Garage::Type>(garage.type));
    }
    for (auto &c : garageData.cars) {
        if (c.modelId == 0) continue;
        auto& car = c;
        glm::quat rotation(
            glm::mat3(glm::cross(car.rotation, glm::vec3(0.f, 0.f, 1.f)),
                      car.rotation, glm::vec3(0.f, 0.f, 1.f)));

        VehicleObject* vehicle =
            state.world->createVehicle(car.modelId, car.position, rotation);
        vehicle->setPrimaryColour(car.colorFG);
        vehicle->setSecondaryColour(car.colorBG);
    }

    for (unsigned g = 0; g < carGenerators.size(); ++g) {
        auto& gen = carGenerators[g];
        state.vehicleGenerators.emplace_back(
            g, gen.position, gen.angle, gen.modelId, gen.colourFG, gen.colourBG,
            gen.force, gen.alarmChance, gen.lockedChance, gen.minDelay,
            gen.maxDelay, gen.timestamp,
            101  /// @todo determine where the remainingSpawns should be
            );
    }

    // Load import / export lists
    state.importExportPortland = garageData.bfImportExportPortland;
    state.importExportShoreside = garageData.bfImportExportShoreside;
    state.importExportUnused = garageData.bfImportExportUnused;

    std::fclose(loadFile);

    return true;
}

bool SaveGame::getSaveInfo(const std::string& file, BasicState* basicState) {
    std::FILE* loadFile = std::fopen(file.c_str(), "rb");

    SaveGameInfo info;
    info.savePath = file;

    // BLOCK 0
    BlockDword blockSize;
    if (fread(&blockSize, sizeof(BlockDword), 1, loadFile) == 0) {
        return false;
    }

    // Read block 0 into state
    if (fread(basicState, sizeof(BasicState), 1, loadFile) == 0) {
        return false;
    }

    std::fclose(loadFile);

    return true;
}

#ifdef RW_WINDOWS
char* readUserPath() {
    LONG retval;
    const char* lpSubKey = "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders";
    const char* lpValueName = "Personal";
    DWORD lpType = REG_SZ;
    HKEY phkResult;
    static char lpData[1000];
    DWORD lpcbData = sizeof(lpData);

    retval = RegOpenKeyEx(HKEY_CURRENT_USER, lpSubKey, 0, KEY_READ, &phkResult);
    if (ERROR_SUCCESS != retval) { return nullptr; }

    retval = RegQueryValueEx(phkResult, lpValueName, NULL, &lpType, (LPBYTE) lpData, &lpcbData);
    if (ERROR_SUCCESS != retval) { return nullptr; }

    retval = RegCloseKey(phkResult);
    if (ERROR_SUCCESS != retval) { return nullptr; }

    return lpData;
}
#endif

std::vector<SaveGameInfo> SaveGame::getAllSaveGameInfo() {
#ifdef RW_WINDOWS
    auto homedir = readUserPath(); // already includes MyDocuments/Documents
#else
    auto homedir = getenv("HOME");
#endif
    if (homedir == nullptr) {
        std::cerr << "Unable to determine home directory" << std::endl;
        return {};
    }
    const char gameDir[] = "GTA3 User Files";

    rwfs::path gamePath(homedir);
    gamePath /= gameDir;

    if (!rwfs::exists(gamePath) || !rwfs::is_directory(gamePath)) return {};

    std::vector<SaveGameInfo> infos;
    for (const rwfs::path& save_path : rwfs::directory_iterator(gamePath)) {
        if (save_path.extension() == ".b") {
            infos.emplace_back(
                SaveGameInfo{save_path.string(), false, BasicState()});
            infos.back().valid =
                getSaveInfo(infos.back().savePath, &infos.back().basicState);
        }
    }

    return infos;
}
