#ifndef RWENGINE_DATA_PEDDATA_HPP
#define RWENGINE_DATA_PEDDATA_HPP
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
    uint32_t id_;

    // Unknown values
    float a_;
    float b_;
    float c_;
    float d_;
    float e_;

    uint32_t threatflags_;
    uint32_t avoidflags_;

public:
    static uint32_t threatFromName(const std::string& name);
};

#endif
