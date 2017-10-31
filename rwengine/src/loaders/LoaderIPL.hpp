#ifndef _RWENGINE_LOADERIPL_HPP_
#define _RWENGINE_LOADERIPL_HPP_

#include <memory>
#include <string>
#include <vector>

#include <data/ZoneData.hpp>

struct InstanceData;

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
    ZoneDataList zones;
};

#endif  // LoaderIPL_h__
