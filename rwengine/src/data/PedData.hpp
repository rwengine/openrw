#ifndef _RWENGINE_PEDDATA_HPP_
#define _RWENGINE_PEDDATA_HPP_

#include <cstdint>
#include <string>
#include <vector>

class PedStats {
public:
    int id_;
    std::string name_;

    float fleedistance_;
    float rotaterate_;
    float fear_;
    float temper_;
    float lawful_;
    float sexy_;
    float attackstrength_;
    float defendweakness_;
    uint32_t flags_;
};
using PedStatsList = std::vector<PedStats>;

class PedRelationship {
public:
    enum {
        THREAT_PLAYER1 = 1,          // Player
        THREAT_PLAYER2 = 2,          // Unused
        THREAT_PLAYER3 = 3,          // Unused
        THREAT_PLAYER4 = 4,          // Unused
        THREAT_CIVMALE = 16,         // Civilan
        THREAT_CIVFEMALE = 32,       // Civilan
        THREAT_COP = 64,             // Police
        THREAT_GANG1 = 128,          // Mafia
        THREAT_GANG2 = 256,          // Triad
        THREAT_GANG3 = 512,          // Diablo
        THREAT_GANG4 = 1024,         // Yakuza
        THREAT_GANG5 = 2048,         // Yardie
        THREAT_GANG6 = 4096,         // Columbian
        THREAT_GANG7 = 8192,         // Hood
        THREAT_GANG8 = 16384,        // Unused
        THREAT_GANG9 = 32768,        // Unused
        THREAT_EMERGENCY = 65536,    // Emergency services
        THREAT_PROSTITUTE = 131072,  // ...
        THREAT_CRIMINAL = 262144,    // Criminals
        THREAT_SPECIAL = 524288,     // SPECIAL
        THREAT_GUN = 1048576,        // Not sure
        THREAT_COP_CAR = 2097152,
        THREAT_FAST_CAR = 4194304,
        THREAT_EXPLOSION = 8388608,  // Explosions
        THREAT_FIREMAN = 16777216,   // Firemen?
        THREAT_DEADPEDS = 33554432,  // Dead bodies
    };

    uint32_t id_ = 0;

    // Unknown values
    float a_ = 0.f;
    float b_ = 0.f;
    float c_ = 0.f;
    float d_ = 0.f;
    float e_ = 0.f;

    uint32_t threatflags_ = 0;
    uint32_t avoidflags_ = 0;

    static uint32_t threatFromName(const std::string& name);
};

using PedGroup = std::vector<uint16_t>;
using PedGroupList = std::vector<PedGroup>;

#endif
