#include "PedData.hpp"
#include <unordered_map>

uint32_t PedRelationship::threatFromName(const std::string &name) {
    static const std::unordered_map<std::string, uint32_t> kThreatMap{
        {"PLAYER1", 1},          // Player
        {"PLAYER2", 2},          // Unused
        {"PLAYER3", 3},          // Unused
        {"PLAYER4", 4},          // Unused
        {"CIVMALE", 16},         // Civilan
        {"CIVFEMALE", 32},       // Civilan
        {"COP", 64},             // Police
        {"GANG1", 128},          // Mafia
        {"GANG2", 256},          // Triad
        {"GANG3", 512},          // Diablo
        {"GANG4", 1024},         // Yakuza
        {"GANG5", 2048},         // Yardie
        {"GANG6", 4096},         // Columbian
        {"GANG7", 8192},         // Hood
        {"GANG8", 16384},        // Unused
        {"GANG9", 32768},        // Unused
        {"EMERGENCY", 65536},    // Emergency services
        {"PROSTITUTE", 131072},  // ...
        {"CRIMINAL", 262144},    // Criminals
        {"SPECIAL", 524288},     // SPECIAL
        {"GUN", 1048576},        // Not sure
        {"COP_CAR", 2097152},
        {"FAST_CAR", 4194304},
        {"EXPLOSION", 8388608},  // Explosions
        {"FIREMAN", 16777216},   // Firemen?
        {"DEADPEDS", 33554432},  // Dead bodies
    };

    return kThreatMap.at(name);
}
