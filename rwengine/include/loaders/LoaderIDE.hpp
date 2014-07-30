#pragma once
#ifndef _LOADERIDE_HPP_
#define _LOADERIDE_HPP_

#include <iostream>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <data/ObjectData.hpp>
#include <objects/VehicleInfo.hpp>
#include <data/PathData.hpp>

class LoaderIDE
{
public:
	enum SectionTypes
	{
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

	std::vector<std::shared_ptr<ObjectData>> OBJSs;
	std::vector<std::shared_ptr<VehicleData>> CARSs;
	std::vector<std::shared_ptr<CharacterData>> PEDSs;
	std::vector<std::shared_ptr<PathData>> PATHs;
	std::vector<std::shared_ptr<CutsceneObjectData>> HIERs;
};

#endif
