#ifndef _LOADERIPL_HPP_
#define _LOADERIPL_HPP_
#include <data/InstanceData.hpp>
#include <data/ZoneData.hpp>

#include <iostream>
#include <memory>
#include <vector>

/**
    \class LoaderIPL
    \brief Loads all data from a IPL file into memory
*/
class LoaderIPL {
public:
    /// Load the IPL data into memory
    bool load(const std::string& filename);

    /// The list of instances from the IPL file
    std::vector<std::shared_ptr<InstanceData>> m_instances;

    /// List of Zones
    std::vector<ZoneData> zones;
};

#endif  // LoaderIPL_h__
