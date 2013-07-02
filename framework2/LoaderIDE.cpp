#include <renderwure/loaders/LoaderIDE.hpp>

#include <fstream>
#include <string>
#include <map>
#include <sstream>
#include <algorithm>

bool LoaderIDE::load(const std::string &filename)
{
	std::ifstream str(filename);

	if ( ! str.is_open())
		return false;

	SectionTypes section = NONE;
	while( ! str.eof()) {
		std::string line;
		getline(str, line);
		line.erase(std::find_if(line.rbegin(), line.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), line.end());

		if ( ! line.empty() && line[0] == '#')
			continue;

		if (line == "end") {
			section = NONE;
		} else if(section == NONE) {
			if (line == "objs") {
				section = OBJS;
			} else if (line == "tobj") {
				section = TOBJ;
			} else if (line == "peds") {
				section = PEDS;
			} else if (line == "cars") {
				section = CARS;
			} else if (line == "hier") {
				section = HIER;
			} else if (line == "2dfx") {
				section = TWODFX;
			} else if (line == "path") {
				section = PATH;
			}
		} else {
			// Remove ALL the whitespace!!
			line.erase(remove_if(line.begin(), line.end(), isspace), line.end());

			std::stringstream strstream(line);

			switch (section) {
			case OBJS: { // Supports Type 1, 2 and 3
				OBJS_t objs;

				std::string id, numClumps, flags,
				            modelName, textureName;
				
				// Read the content of the line
				getline(strstream, id, ',');
				getline(strstream, modelName, ',');
				getline(strstream, textureName, ',');
				getline(strstream, numClumps, ',');

				objs.numClumps = atoi(numClumps.c_str());
				for (size_t i = 0; i < objs.numClumps; i++) {
					std::string drawDistance;
					getline(strstream, drawDistance, ',');
					objs.drawDistance[i] = atoi(drawDistance.c_str());
				}

				getline(strstream, flags, ',');

				// Put stuff in our struct
				objs.ID          = atoi(id.c_str());
				objs.flags       = atoi(flags.c_str());
				objs.modelName   = modelName;
				objs.textureName = textureName;

				OBJSs.push_back(objs);
				break;
			}
			case CARS: {
				CARS_t cars;

				std::string id, type, classType, frequency, lvl,
				            comprules, wheelModelID, wheelScale;

				getline(strstream, id, ',');
				getline(strstream, cars.modelName, ',');
				getline(strstream, cars.textureName, ',');
				getline(strstream, type, ',');
				getline(strstream, cars.handlingID, ',');
				getline(strstream, cars.gameName, ',');
				getline(strstream, classType, ',');
				getline(strstream, frequency, ',');
				getline(strstream, lvl, ',');
				getline(strstream, comprules, ',');
				getline(strstream, wheelModelID, ',');
				getline(strstream, wheelScale, ',');

				cars.ID = atoi(id.c_str());
				cars.frequency = atoi(frequency.c_str());
				cars.lvl = atoi(lvl.c_str());
				cars.comprules = atoi(comprules.c_str());

				if (type == "car") {
					cars.type = CAR;
					cars.wheelModelID = atoi(wheelModelID.c_str());
					cars.wheelScale = atof(wheelScale.c_str());
				} else if (type == "boat") {
					cars.type = BOAT;
				} else if (type == "train") {
					cars.type = TRAIN;
					cars.modelLOD = atoi(wheelModelID.c_str());
				} else if (type == "plane") {
					cars.type = PLANE;
				} else if (type == "heli") {
					cars.type = HELI;
				}

				const std::map<VehicleClass, std::string> classTypes{
					{IGNORE,      "ignore"},
					{NORMAL,      "normal"},
					{POORFAMILY,  "poorfamily"},
					{RICHFAMILY,  "richfamily"},
					{EXECUTIVE,   "executive"},
					{WORKER,      "worker"},
					{BIG,         "big"},
					{TAXI,        "taxi"},
					{MOPED,       "moped"},
					{MOTORBIKE,   "motorbike"},
					{LEISUREBOAT, "leisureboat"},
					{WORKERBOAT,  "workerboat"},
					{BICYCLE,     "bicycle"},
					{ONFOOT,      "onfoot"},
				};
				for (auto &a : classTypes) {
					if (classType == a.second) {
						cars.classType = a.first;
						break;
					}
				}

				CARSs.push_back(cars);
				break;
			}
			case PEDS: {
				PEDS_t peds;

				std::string id, driveMask;

				getline(strstream, id, ',');
				getline(strstream, peds.modelName, ',');
				getline(strstream, peds.textureName, ',');
				getline(strstream, peds.type, ',');
				getline(strstream, peds.behaviour, ',');
				getline(strstream, peds.animGroup, ',');
				getline(strstream, driveMask, ',');

				peds.ID = atoi(id.c_str());
				peds.driveMask = atoi(driveMask.c_str());

				PEDSs.push_back(peds);
				break;
			}
			}
		}

	}

	return true;
}