#include "engine/SaveGame.hpp"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <filesystem>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <rw/debug.hpp>

#include "data/ZoneData.hpp"
#include "engine/GameData.hpp"
#include "engine/GameState.hpp"
#include "engine/GameWorld.hpp"
#include "objects/CharacterObject.hpp"
#include "objects/GameObject.hpp"
#include "objects/InstanceObject.hpp"
#include "objects/VehicleObject.hpp"
#include "script/SCMFile.hpp"
#include "script/ScriptMachine.hpp"
#include "script/ScriptTypes.hpp"

namespace {
    constexpr int kNrOfWeapons = 13;
    constexpr int kNrOfStoredCars = 18;
    constexpr int kNrOfPickups = 336;
    constexpr int kNrOfBlips = 32;
    constexpr int kNrOfNavZones = 50;
    constexpr int kNrOfDayNightInfo = 100;
    constexpr int kNrOfMapZones = 25;
    constexpr int kNrOfAudioZones = 36;
    constexpr int kNrOfGangs = 9;
    constexpr int kNrOfPedTypes = 23;
}

// Original save game file data structures
typedef uint16_t BlockWord;
typedef uint32_t BlockDword;
typedef BlockDword BlockSize;

struct Block0ContactInfo {
    BlockDword missionFlag;
    BlockDword baseBrief;
};

struct Block0BuildingSwap {
    BlockDword type;
    BlockDword handle;
    BlockDword newModel;
    BlockDword oldModel;
};

struct Block0InvisibilitySettings {
    BlockDword type;
    BlockDword handle;
};

struct Block0RunningScript {
    uint32_t nextPointer;
    uint32_t prevPointer;
    char name[8];
    BlockDword programCounter;
    BlockDword stack[4];
    BlockDword unknown0;
    BlockDword unknown1;
    BlockWord stackCounter;
    BlockWord unknown2;
    SCMByte variables[16 * 4];
    BlockDword timerA;
    BlockDword timerB;
    uint8_t ifFlag;
    uint8_t unknown3;
    uint8_t unknown4;
    uint8_t _align0;
    BlockDword wakeTimer;
    BlockWord ifNumber;  // ?
    uint8_t unknown[6];
};

struct Block0ScriptData {
    BlockDword onMissionOffset;
    Block0ContactInfo contactInfo[16];
    uint8_t unknown[0x100];
    BlockDword lastMissionPassedTime;
    Block0BuildingSwap buildingSwap[25];
    Block0InvisibilitySettings invisibilitySettings[20];
    uint8_t scriptRunning;
    uint8_t _align0[3];
    BlockDword mainSize;
    BlockDword largestMissionSize;
    BlockWord missionCount;
    uint8_t _align1[2];
};

struct StructWeaponSlot {
    BlockDword weaponId;
    BlockDword unknown0;
    BlockDword inClip;
    BlockDword totalBullets;
    BlockDword unknown1;
    BlockDword unknown2;
};
struct StructPed {
    uint8_t unknown0_[52];
    glm::vec3 position{};
    uint8_t unknown1[640];
    float health;
    float armour;
    uint8_t unknown2[148];
    std::array<StructWeaponSlot, kNrOfWeapons> weapons;
    uint8_t unknown3[348];
};

struct Block1PlayerPed {
    BlockDword unknown0;
    BlockWord unknown1;
    BlockDword reference;
    StructPed info;
    BlockDword maxWantedLevel;
    BlockDword maxChaosLevel;
    uint8_t modelName[24];
    uint8_t align[2];
};

struct StructStoredCar {
    BlockDword modelId;
    glm::vec3 position{};
    glm::vec3 rotation{};
    BlockDword immunities;
    uint8_t colorFG;
    uint8_t colorBG;
    uint8_t radio;
    uint8_t variantA;
    uint8_t variantB;
    uint8_t bombType;
    uint8_t align0[2];

    // TODO Migrate to more available location (GameConstants?)
    enum /*VehicleImmunities*/ {
        Bulletproof = 1 << 0,
        Fireproof = 1 << 1,
        Explosionproof = 1 << 2,
        CollisionProof = 1 << 3,
        UnknownProof = 1 << 4
    };
    enum /*VehicleBombType*/ {
        NoBomb = 0,
        TimerBomb = 1,
        IgnitionBomb = 2,
        RemoteBomb = 3,
        TimerBombArmed = 4,
        IgnitionBombArmed = 5
    };
};

struct StructGarage {
    uint8_t type;
    uint8_t unknown0;
    uint8_t unknown1;
    uint8_t unknown2;
    uint8_t unknown3;
    uint8_t unknown4;
    uint8_t unknown5;
    uint8_t align0[2];
    BlockDword unknown6;
    BlockDword unknown7;
    uint8_t unknown8;
    uint8_t unknown9;
    uint8_t unknown10;
    uint8_t unknown11;
    uint8_t unknown12;
    uint8_t unknown13;
    uint8_t unknown14;
    uint8_t align1;
    float x1;
    float x2;
    float y1;
    float y2;
    float z1;
    float z2;
    float doorOpenStart;
    float doorOpenAngle;
    glm::vec2 unknownCoord1{};
    glm::vec2 unknownCoord2{};
    float doorAZ;
    float doorBZ;
    BlockDword unknown15;
    uint8_t unknown16;
    uint8_t align2[3];
    BlockDword unknown17;
    BlockDword unknown18;
    BlockDword unknown19;
    float unknown20;
    float unknown21;
    float unknown22;
    float unknown23;
    float unknown24;
    float unknown25;
    BlockDword unknown26;
    uint8_t unknown27;
    uint8_t unknown28;
    uint8_t unknown29;
    uint8_t unknown30;
    uint8_t unknown31;
    uint8_t unknown32;
    uint8_t align3[2];
};

struct Block2GarageData {
    BlockDword garageCount;
    BlockDword freeBombs;
    BlockDword freeResprays;
    BlockDword unknown0;
    BlockDword unknown1;
    BlockDword unknown2;
    BlockDword bfImportExportPortland;
    BlockDword bfImportExportShoreside;
    BlockDword bfImportExportUnused;
    BlockDword GA_21lastTime;
    std::array<StructStoredCar, kNrOfStoredCars> cars;
};

struct Block3VehicleState {
    uint8_t unknown1[52];
    glm::vec3 position{};
    uint8_t unknown2[1384];
};

struct Block3Vehicle {
    BlockDword unknown1;
    BlockWord modelId;
    BlockDword unknown2;
    Block3VehicleState state;
};

struct Block3BoatState {
    uint8_t unknown1[52];
    glm::vec3 position{};
    uint8_t unknown2[1092];
};

struct Block3Boat {
    BlockDword unknown1;
    BlockWord modelId;
    BlockDword unknown2;
    Block3BoatState state;
};

struct Block4Object {
    BlockWord modelId;
    BlockDword reference;
    glm::vec3 position{};
    int8_t rotation[9];  /// @todo Confirm that this is: right, forward, down
    uint8_t unknown1[3];
    float unknown2;
    float unknown3[3];
    uint8_t unknown4[12];
    uint8_t unknown5[8];
    float unknown6;
    uint8_t unknown7[2];
    BlockDword unknown8;
    BlockDword unknown9;
    BlockDword unknown10;
};

struct Block6Crane {
    BlockDword reference;
    BlockDword hookReference;
    BlockDword audioReference;
    float x1Pickup;
    float y1Pickup;
    float x2Pickup;
    float y2Pickup;
    glm::vec3 dropoff{};
    float dropoffHeadingRads;
    float pickupArmRads;
    float dropoffArmRads;
    float armPickupDistance;
    float armDropoffDistance;
    float armPickupHeight;
    float armDropoffHeight;
    float armCurrentRads;
    float armCurrentDistance;
    float armCurrentHeight;
    glm::vec3 hookInitialPosition{};
    glm::vec3 hookCurrentPosition{};
    float unknown1[2];
    BlockDword vehiclePtr;
    BlockDword gameTime;
    uint8_t activity;
    uint8_t status;
    uint8_t vehiclesCollected;
    uint8_t isCrusher;
    uint8_t isMilitary;
    uint8_t unknown2;
    uint8_t isNotdoc_crane_cab;
    uint8_t padding;
};

struct Block6Data {
    BlockDword numCranes;
    BlockDword militaryCollected;
    Block6Crane cranes[8];
};

struct Block7Pickup {
    uint8_t type;
    uint8_t unknown1;
    BlockWord ammo;
    BlockDword objectRef;
    BlockDword regenTime;
    BlockWord modelId;
    BlockWord flags;
    glm::vec3 position{};
};

struct Block7Data {
    std::array<Block7Pickup, kNrOfPickups> pickups;
    BlockWord pickupIndex;
    uint8_t align[2];
    BlockWord collectedPickups[20];
};

struct Block8Data {
    BlockDword numPayphones;
    BlockDword numActivePayphones;
};

struct Block8Payphone {
    glm::vec3 position{};
    BlockDword messagePtr[6];
    BlockDword messageEndTime;
    BlockDword staticIndex;
    BlockDword state;
    uint8_t playerInRange;
    uint8_t align[3];
};

struct Block9Restart {
    glm::vec3 position{};
    float angle;
};

struct Block9Data {
    Block9Restart hospitalRestarts[8];
    Block9Restart policeRestarts[8];
    BlockWord numHospitals;
    BlockWord numPolice;
    uint8_t overrideFlag;
    uint8_t align[3];
    Block9Restart overrideRestart;
    uint8_t deathFadeInFlag;
    uint8_t arrestFadeInFlag;
    uint8_t hospitalLevelOverride;
    uint8_t policeLevelOverride;
};

struct Block10Blip {
    BlockDword color;
    BlockDword type;
    BlockDword entityHandle;
    float unknown1;
    float unknown2;
    glm::vec3 position{};
    BlockWord unknown3;
    uint8_t brightness;
    uint8_t unknown4;
    float unknown5;
    BlockWord scale;
    BlockWord display;
    BlockWord sprite;
    BlockWord align;
};

struct Block10Data {
    std::array<Block10Blip, kNrOfBlips> blips;
};

struct Block11Zone {
    char name[8];
    glm::vec3 coordA{};
    glm::vec3 coordB{};
    BlockDword type;
    BlockDword level;
    BlockWord dayZoneInfo;
    BlockWord nightZoneInfo;
    BlockDword childZone;
    BlockDword parentZone;
    BlockDword siblingZone;
};
struct Block11ZoneInfo {
    BlockWord density;
    BlockWord unknown1[16];
    BlockWord peddensity;
    BlockWord copdensity;
    BlockWord gangpeddensity[9];
    BlockWord pedgroup;
};
struct Block11AudioZone {
    BlockWord zoneId;
};

struct Block11Data {
    BlockDword currentZone;
    BlockDword currentLevel;
    BlockWord findIndex;
    BlockWord align;
    std::array<Block11Zone, kNrOfNavZones> navZones;
    std::array<Block11ZoneInfo, kNrOfDayNightInfo> dayNightInfo;
    BlockWord numNavZones;
    BlockWord numZoneInfos;
    std::array<Block11Zone, kNrOfMapZones> mapZones;
    std::array<Block11AudioZone, kNrOfAudioZones> audioZones;
    BlockWord numMapZones;
    BlockWord numAudioZones;
};

struct Block12Gang {
    BlockDword carModelId;
    uint8_t pedModelOverrideFlag;
    uint8_t align[3];
    BlockDword weaponPrimary;
    BlockDword weaponSecondary;
};

struct Block12Data {
    std::array<Block12Gang, kNrOfGangs> gangs;
};

struct Block13CarGenerator {
    BlockDword modelId;
    glm::vec3 position{};
    float angle;
    BlockWord colourFG;
    BlockWord colourBG;
    uint8_t force;
    uint8_t alarmChance;
    uint8_t lockedChance;
    uint8_t align;
    BlockWord minDelay;
    BlockWord maxDelay;
    BlockDword timestamp;
    BlockDword unknown1;
    BlockDword unknown2;
    float unknown3;
    float unknown4;
    float unknown5;
    float unknown6;
    float unknown7;
    float unknown8;
    BlockDword unknown9;
};

struct Block13Data {
    BlockDword blockSize;
    BlockDword generatorCount;
    BlockDword activeGenerators;
    uint8_t counter;
    uint8_t generateNearPlayerCounter;
    uint8_t align[2];
    BlockDword generatorSize;
};

struct Block14Particle {
    uint8_t unknown[0x88];
};

struct Block15AudioObject {
    BlockDword index;
    BlockWord soundIndex;
    uint8_t align[2];
    glm::vec3 position{};
    BlockDword unknown1;
};

struct Block18Data {
    uint8_t unknown1[200];
};

struct Block19PedType {
    BlockDword bitstring_;
    float unknown2;
    float unknown3;
    float unknown4;
    float fleedistance;
    float headingchangerate;
    BlockDword threatflags_;
    BlockDword avoidflags_;
};
struct Block19Data {
    std::array<Block19PedType, kNrOfPedTypes> types;
};

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
                  << ped.info.armour << ")" << '\n';
        std::cout << "Player model: " << ped.modelName << '\n';
        for (const auto &wep : players[p].info.weapons) {
            std::cout << "ID " << wep.weaponId << " " << wep.inClip << " "
                      << wep.totalBullets << '\n';
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
    std::cout << "Garages: " << garageData.garageCount << '\n';
    std::cout << "Bombs Free: " << garageData.freeBombs << '\n';
    std::cout << "Sprays Free: " << garageData.freeResprays << '\n';
    std::cout << "Portland IE: " << garageData.bfImportExportPortland
              << '\n';
    std::cout << "Shoreside IE: " << garageData.bfImportExportShoreside
              << '\n';
    std::cout << "GA21 last shown: " << garageData.GA_21lastTime << '\n';
    for (const auto &car : garageData.cars) {
        if (car.modelId == 0) continue;
        std::cout << " " << car.modelId << " " << uint16_t(car.colorFG) << "/"
                  << uint16_t(car.colorBG) << " " << car.immunities
                  << '\n';
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
                  << '\n';
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
                  << '\n';
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
    std::cout << "Objects " << objectCount << '\n';
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
                  << '\n';
    }
    std::cout << '\n';
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
    std::cout << "Cranes: " << craneData.numCranes << '\n';
    for (size_t c = 0; c < craneData.numCranes; ++c) {
        Block6Crane& crane = craneData.cranes[c];
        std::cout << "pickup " << crane.x1Pickup << " " << crane.y1Pickup << " "
                  << crane.x2Pickup << " " << crane.y2Pickup << '\n';
        std::cout << "vehicles collected " << uint16_t(crane.vehiclesCollected)
                  << '\n';
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
                  << pickup.position.y << " " << pickup.position.z << '\n';
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
    std::cout << "Payphones: " << payphoneData.numPayphones << '\n';
    for (const auto& payphone : payphones) {
        std::cout << " " << uint16_t(payphone.state) << " " << payphone.position.x
                  << " " << payphone.position.y << " " << payphone.position.z
                  << '\n';
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
              << " police: " << restartData.numPolice << '\n';
    for (int s = 0; s < restartData.numHospitals; ++s) {
        Block9Restart& p = restartData.hospitalRestarts[s];
        std::cout << " H " << p.position.x << " " << p.position.y << " "
                  << p.position.z << '\n';
    }
    for (int s = 0; s < restartData.numPolice; ++s) {
        Block9Restart& p = restartData.policeRestarts[s];
        std::cout << " P " << p.position.x << " " << p.position.y << " "
                  << p.position.z << '\n';
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
                  << blip.position.z << '\n';
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
              << zoneData.numAudioZones << '\n';
    for (int z = 0; z < zoneData.numNavZones; ++z) {
        Block11Zone& zone = zoneData.navZones[z];
        std::cout << " " << zone.name << '\n';
        auto& dayinfo = zoneData.dayNightInfo[zone.dayZoneInfo];
        std::cout << "  DAY " << dayinfo.density << " " << dayinfo.peddensity
                  << " " << dayinfo.copdensity << " "
                  << " [";
        for (BlockDword gang : dayinfo.gangpeddensity) {
            std::cout << " " << gang;
        }
        std::cout << "] " << dayinfo.pedgroup << '\n';
        for (BlockDword dw : dayinfo.unknown1) {
            std::cout << " " << dw;
        }
        std::cout << '\n';

        auto& nightinfo = zoneData.dayNightInfo[zone.nightZoneInfo];
        std::cout << "  NIGHT " << nightinfo.density << " "
                  << nightinfo.peddensity << " " << nightinfo.copdensity << " "
                  << " [";
        for (BlockDword gang : nightinfo.gangpeddensity) {
            std::cout << " " << gang;
        }
        std::cout << "] " << nightinfo.pedgroup << '\n';
        for (BlockDword dw : nightinfo.unknown1) {
            std::cout << " " << dw;
        }
        std::cout << '\n';
    }
    for (int z = 0; z < zoneData.numMapZones; ++z) {
        Block11Zone& zone = zoneData.mapZones[z];
        std::cout << " " << zone.name << '\n';
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
                  << gang.weaponSecondary << '\n';
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
              << '\n';
    for (size_t g = 0; g < carGeneratorData.generatorCount; ++g) {
        Block13CarGenerator& gen = carGenerators[g];
        std::cout << " " << gen.modelId << " " << gen.position.x << " "
                  << gen.position.y << " " << gen.position.z << '\n';
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
    std::cout << "particles: " << particleCount << '\n';
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
    std::cout << "Audio Objects: " << audioCount << '\n';
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
              << state.playerInfo.displayedMoney << ")" << '\n';
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
    std::cout << "Player kills: " << state.gameStats.playerKills << '\n';
    std::cout << "longest flight " << state.gameStats.longestDodoFlight
              << '\n';
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
        std::cout << '\n';
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
        std::cout << ply.reference << '\n';
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
                                  static_cast<GarageType>(garage.type));
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
        std::cerr << "Unable to determine home directory" << '\n';
        return {};
    }
    const char gameDir[] = "GTA3 User Files";

    std::filesystem::path gamePath(homedir);
    gamePath /= gameDir;

    if (!std::filesystem::exists(gamePath) || !std::filesystem::is_directory(gamePath)) return {};

    std::vector<SaveGameInfo> infos;
    for (const std::filesystem::path& save_path : std::filesystem::directory_iterator(gamePath)) {
        if (save_path.extension() == ".b") {
            infos.emplace_back(
                SaveGameInfo{save_path.string(), false, BasicState()});
            infos.back().valid =
                getSaveInfo(infos.back().savePath, &infos.back().basicState);
        }
    }

    return infos;
}
