#ifndef _RWENGINE_SAVEGAMETYPES_HPP_
#define _RWENGINE_SAVEGAMETYPES_HPP_
#include <cstdint>

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

namespace SaveGameSerialization {
template <class T>
bool serialize(T& s, Block1PlayerPed& p) {
    return
        serialize(s, p.unknown0) &&
        serialize(s, p.unknown1) &&
        serialize(s, p.reference) &&
        serialize(s, p.info) &&
        serialize(s, p.maxWantedLevel) &&
        serialize(s, p.maxChaosLevel) &&
        serialize(s, p.modelName);
}

template <class T>
bool serialize(T& s, Block2GarageData& v) {
    return
        serialize(s, v.garageCount) &&
        serialize(s, v.freeBombs) &&
        serialize(s, v.freeResprays) &&
        serialize(s, v.unknown0) &&
        serialize(s, v.unknown1) &&
        serialize(s, v.unknown2) &&
        serialize(s, v.bfImportExportPortland) &&
        serialize(s, v.bfImportExportShoreside) &&
        serialize(s, v.bfImportExportUnused) &&
        serialize(s, v.GA_21lastTime) &&
        serialize(s, v.cars);
}

template <class T>
bool serialize(T& s, Block3Vehicle& v) {
    return
        serialize(s, v.unknown1) &&
        serialize(s, v.modelId) &&
        serialize(s, v.unknown2) &&
        serialize(s, v.state);
}

template <class T>
bool serialize(T& s, Block3Boat& v) {
    return
        serialize(s, v.unknown1) &&
        serialize(s, v.modelId) &&
        serialize(s, v.unknown2) &&
        serialize(s, v.state);
}

template <class T>
bool serialize(T& s, Block4Object& v) {
    return
        serialize(s, v.modelId) &&
        serialize(s, v.reference) &&
        serialize(s, v.position) &&
        serialize(s, v.rotation) &&
        serialize(s, v.unknown1) &&
        serialize(s, v.unknown2) &&
        serialize(s, v.unknown3) &&
        serialize(s, v.unknown4) &&
        serialize(s, v.unknown5) &&
        serialize(s, v.unknown6) &&
        serialize(s, v.unknown7) &&
        serialize(s, v.unknown8) &&
        serialize(s, v.unknown9) &&
        serialize(s, v.unknown10);
}
}

#endif //namespace _RWENGINE_SAVEGAMETYPES_HPP_
