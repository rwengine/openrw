#ifndef RWENGINE_LOADERIDE_HPP
#define RWENGINE_LOADERIDE_HPP

#include <data/ModelData.hpp>
#include <data/PathData.hpp>
#include <glm/glm.hpp>
#include <iostream>
#include <map>
#include <objects/VehicleInfo.hpp>

class LoaderIDE {
public:
    enum SectionTypes {
        NONE,
        OBJS,
        TOBJ,
        PEDS,
        CARS,
        HIER,
        TWODFX,
        PATH,
    };

    // Load the IDE data into memory
    bool load(const std::string& filename);

    /**
     * @brief objects loaded during the call to load()
     */
    std::map<ModelID, std::unique_ptr<BaseModelInfo>> objects;
};

#endif
