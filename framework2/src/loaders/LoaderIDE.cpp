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
				std::shared_ptr<ObjectData> objs(new ObjectData);

				std::string id, numClumps, flags,
				            modelName, textureName;
				
				// Read the content of the line
				getline(strstream, id, ',');
				getline(strstream, modelName, ',');
				getline(strstream, textureName, ',');
				getline(strstream, numClumps, ',');

				objs->numClumps = atoi(numClumps.c_str());
				for (size_t i = 0; i < objs->numClumps; i++) {
					std::string drawDistance;
					getline(strstream, drawDistance, ',');
					objs->drawDistance[i] = atoi(drawDistance.c_str());
				}

				getline(strstream, flags, ',');

				// Put stuff in our struct
				objs->ID          = atoi(id.c_str());
				objs->flags       = atoi(flags.c_str());
				objs->modelName   = modelName;
				objs->textureName = textureName;
				objs->LOD         = modelName.find("LOD",0,3) != modelName.npos;

				OBJSs.push_back(objs);
				break;
			}
			case CARS: {
				std::shared_ptr<CarData> cars(new CarData);

				std::string id, type, classType, frequency, lvl,
				            comprules, wheelModelID, wheelScale;

				getline(strstream, id, ',');
				getline(strstream, cars->modelName, ',');
				getline(strstream, cars->textureName, ',');
				getline(strstream, type, ',');
				getline(strstream, cars->handlingID, ',');
				getline(strstream, cars->gameName, ',');
				getline(strstream, classType, ',');
				getline(strstream, frequency, ',');
				getline(strstream, lvl, ',');
				getline(strstream, comprules, ',');
				getline(strstream, wheelModelID, ',');
				getline(strstream, wheelScale, ',');

				cars->ID = atoi(id.c_str());
				cars->frequency = atoi(frequency.c_str());
				cars->lvl = atoi(lvl.c_str());
				cars->comprules = atoi(comprules.c_str());

				if (type == "car") {
					cars->type = CarData::CAR;
					cars->wheelModelID = atoi(wheelModelID.c_str());
					cars->wheelScale = atof(wheelScale.c_str());
				} else if (type == "boat") {
					cars->type = CarData::BOAT;
				} else if (type == "train") {
					cars->type = CarData::TRAIN;
					cars->modelLOD = atoi(wheelModelID.c_str());
				} else if (type == "plane") {
					cars->type = CarData::PLANE;
				} else if (type == "heli") {
					cars->type = CarData::HELI;
				}

				const std::map<CarData::VehicleClass, std::string> classTypes{
					{CarData::IGNORE,      "ignore"},
					{CarData::NORMAL,      "normal"},
					{CarData::POORFAMILY,  "poorfamily"},
					{CarData::RICHFAMILY,  "richfamily"},
					{CarData::EXECUTIVE,   "executive"},
					{CarData::WORKER,      "worker"},
					{CarData::BIG,         "big"},
					{CarData::TAXI,        "taxi"},
					{CarData::MOPED,       "moped"},
					{CarData::MOTORBIKE,   "motorbike"},
					{CarData::LEISUREBOAT, "leisureboat"},
					{CarData::WORKERBOAT,  "workerboat"},
					{CarData::BICYCLE,     "bicycle"},
					{CarData::ONFOOT,      "onfoot"},
				};
				for (auto &a : classTypes) {
					if (classType == a.second) {
						cars->classType = a.first;
						break;
					}
				}

				CARSs.push_back(cars);
				break;
			}
			case PEDS: {
				std::shared_ptr<CharacterData> peds(new CharacterData);

				std::string id, driveMask;

				getline(strstream, id, ',');
				getline(strstream, peds->modelName, ',');
				getline(strstream, peds->textureName, ',');
				getline(strstream, peds->type, ',');
				getline(strstream, peds->behaviour, ',');
				getline(strstream, peds->animGroup, ',');
				getline(strstream, driveMask, ',');

				peds->ID = atoi(id.c_str());
				peds->driveMask = atoi(driveMask.c_str());

				PEDSs.push_back(peds);
				break;
			}
			case PATH: {
				std::shared_ptr<PathData> path(new PathData);

				std::string type;
				getline(strstream, type, ',');
				if( type == "ped" ) {
						path->type = PathData::PATH_PED;
				}
				else if( type == "car") {
						path->type = PathData::PATH_CAR;
				}

				std::string id;
				getline(strstream, id, ',');
				path->ID = atoi(id.c_str());

				getline(strstream, path->modelName);

				std::string linebuff, buff;
				for( size_t p = 0; p < 12; ++p ) {
						PathNode node;

						getline(str, linebuff);
						std::stringstream buffstream(linebuff);

						getline(buffstream, buff, ',');
						switch(atoi(buff.c_str())) {
							case 0:
								node.type = PathNode::EMPTY;
								break;
							case 2:
								node.type = PathNode::INTERNAL;
								break;
							case 1:
								node.type = PathNode::EXTERNAL;
								break;
						}

						if( node.type == PathNode::EMPTY ) {
								continue;
						}

						getline(buffstream, buff, ',');
						node.next = atoi(buff.c_str());

						getline(buffstream, buff, ','); // "Always 0"

						getline(buffstream, buff, ',');
						node.position.x = atof(buff.c_str()) * 1/16.f;

						getline(buffstream, buff, ',');
						node.position.y = atof(buff.c_str()) * 1/16.f;

						getline(buffstream, buff, ',');
						node.position.z = atof(buff.c_str()) * 1/16.f;

						getline(buffstream, buff, ',');
						node.size = atof(buff.c_str()) * 1/16.f;

						getline(buffstream, buff, ',');
						node.other_thing = atoi(buff.c_str());

						getline(buffstream, buff, ',');
						node.other_thing2 = atoi(buff.c_str());

						path->nodes.push_back(node);
					}

					PATHs.push_back(path);

					break;
				}
			}
		}

	}

	return true;
}
