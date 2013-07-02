#pragma once
#ifndef _LOADERIDE_HPP_
#define _LOADERIDE_HPP_

#include <iostream>
#include <vector>
#include <glm/glm.hpp>

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

	struct OBJS_t
	{
		uint16_t ID;
		std::string modelName;
		std::string textureName;
		size_t numClumps;
		float drawDistance[3];
		int32_t flags;
		bool LOD;
	};	

	enum VehicleClass
	{
		IGNORE      = 0,
		NORMAL      = 1,
		POORFAMILY  = 1 << 1,
		RICHFAMILY  = 1 << 2,
		EXECUTIVE   = 1 << 3,
		WORKER      = 1 << 4,
		BIG         = 1 << 5,
		TAXI        = 1 << 6,
		MOPED       = 1 << 7,
		MOTORBIKE   = 1 << 8,
		LEISUREBOAT = 1 << 9,
		WORKERBOAT  = 1 << 10,
		BICYCLE     = 1 << 11,
		ONFOOT      = 1 << 12,
	};
	enum VehicleType {
		CAR,
		BOAT,
		TRAIN,
		PLANE,
		HELI,
	};
	struct CARS_t
	{
		uint16_t ID;
		std::string modelName;
		std::string textureName;
		VehicleType type;
		std::string handlingID;
		std::string gameName;
		VehicleClass classType;
		uint8_t frequency; // big enough int type?
		uint8_t lvl; // big enough int type?
		uint16_t comprules;
		union { // big enough int types?
			uint16_t wheelModelID; // used only when type == CAR
			int16_t modelLOD; // used only when type == PLANE
		};
		float wheelScale; // used only when type == CAR
	};

	struct PEDS_t
	{
		uint16_t ID;
		std::string modelName;
		std::string textureName;
		std::string type;
		std::string behaviour;
		std::string animGroup;
		uint8_t driveMask;
	};

	// Load the IDE data into memory
	bool load(const std::string& filename);

	std::vector<OBJS_t> OBJSs;
	std::vector<CARS_t> CARSs;
	std::vector<PEDS_t> PEDSs;
};

#endif
