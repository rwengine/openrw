#pragma once
#ifndef _LOADERIDE_HPP_
#define _LOADERIDE_HPP_

#include <data/ObjectData.hpp>
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
    std::map<ObjectID, ObjectInformationPtr> objects;

    /*std::vector<std::shared_ptr<ObjectData>> OBJSs;
    std::vector<std::shared_ptr<VehicleData>> CARSs;
    std::vector<std::shared_ptr<CharacterData>> PEDSs;
    std::vector<std::shared_ptr<CutsceneObjectData>> HIERs;
    std::vector<std::shared_ptr<PathData>> PATHs;*/
};

#endif
