#include "PedData.hpp"
#include <unordered_map>

uint32_t PedRelationship::threatFromName(const std::string& name) {
    static const std::unordered_map<std::string, uint32_t> kThreatMap{
        {"PLAYER1", THREAT_PLAYER1},        // Player
        {"PLAYER2", THREAT_PLAYER2},        // Unused
        {"PLAYER3", THREAT_PLAYER3},        // Unused
        {"PLAYER4", THREAT_PLAYER4},        // Unused
        {"CIVMALE", THREAT_CIVMALE},        // Civilan
        {"CIVFEMALE", THREAT_CIVFEMALE},    // Civilan
        {"COP", THREAT_COP},                // Police
        {"GANG1", THREAT_GANG1},            // Mafia
        {"GANG2", THREAT_GANG2},            // Triad
        {"GANG3", THREAT_GANG3},            // Diablo
        {"GANG4", THREAT_GANG4},            // Yakuza
        {"GANG5", THREAT_GANG5},            // Yardie
        {"GANG6", THREAT_GANG6},            // Columbian
        {"GANG7", THREAT_GANG7},            // Hood
        {"GANG8", THREAT_GANG8},            // Unused
        {"GANG9", THREAT_GANG9},            // Unused
        {"EMERGENCY", THREAT_EMERGENCY},    // Emergency services
        {"PROSTITUTE", THREAT_PROSTITUTE},  // ...
        {"CRIMINAL", THREAT_CRIMINAL},      // Criminals
        {"SPECIAL", THREAT_SPECIAL},        // SPECIAL
        {"GUN", THREAT_GUN},                // Not sure
        {"COP_CAR", THREAT_COP_CAR},
        {"FAST_CAR", THREAT_FAST_CAR},
        {"EXPLOSION", THREAT_EXPLOSION},  // Explosions
        {"FIREMAN", THREAT_FIREMAN},      // Firemen?
        {"DEADPEDS", THREAT_DEADPEDS},    // Dead bodies
    };

    return kThreatMap.at(name);
}
