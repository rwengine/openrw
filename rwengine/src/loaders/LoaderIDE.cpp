#include <loaders/LoaderIDE.hpp>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <functional>
#include <map>
#include <sstream>
#include <string>

bool LoaderIDE::load(const std::string &filename, const PedStatsList &stats) {
    std::ifstream str(filename);

    if (!str.is_open()) return false;

    auto find_stat_id = [&](const std::string &name) {
        auto it =
            std::find_if(stats.begin(), stats.end(),
                         [&](const PedStats &a) { return a.name_ == name; });
        if (it == stats.end()) {
            return -1;
        }
        return it->id_;
    };

    SectionTypes section = NONE;
    while (!str.eof()) {
        std::string line;
        getline(str, line);
        line.erase(std::find_if(line.rbegin(), line.rend(),
                                std::not1(std::ptr_fun<int, int>(std::isspace)))
                       .base(),
                   line.end());

        if (!line.empty() && line[0] == '#') continue;

        if (line == "end") {
            section = NONE;
        } else if (section == NONE) {
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
            line.erase(remove_if(line.begin(), line.end(), isspace),
                       line.end());

            std::stringstream strstream(line);
            std::string buff;

            switch (section) {
                default:
                    break;
                case OBJS:
                case TOBJ: {  // Supports Type 1, 2 and 3
                    auto objs = std::make_unique<SimpleModelInfo>();

                    getline(strstream, buff, ',');
                    objs->setModelID(atoi(buff.c_str()));

                    getline(strstream, objs->name, ',');
                    getline(strstream, objs->textureslot, ',');

                    getline(strstream, buff, ',');
                    objs->setNumAtomics(atoi(buff.c_str()));

                    for (int i = 0; i < objs->getNumAtomics(); i++) {
                        getline(strstream, buff, ',');
                        objs->setLodDistance(i, atof(buff.c_str()));
                    }

                    objs->determineFurthest();

                    getline(strstream, buff, ',');
                    objs->flags = atoi(buff.c_str());

                    // Keep reading TOBJ data
                    if (section == LoaderIDE::TOBJ) {
                        getline(strstream, buff, ',');
                        objs->timeOn = atoi(buff.c_str());
                        getline(strstream, buff, ',');
                        objs->timeOff = atoi(buff.c_str());
                    } else {
                        objs->timeOn = 0;
                        objs->timeOff = 24;
                    }

                    objects.emplace(objs->id(), std::move(objs));
                    break;
                }
                case CARS: {
                    auto cars = std::make_unique<VehicleModelInfo>();

                    getline(strstream, buff, ',');
                    cars->setModelID(std::atoi(buff.c_str()));

                    getline(strstream, cars->name, ',');
                    getline(strstream, cars->textureslot, ',');

                    getline(strstream, buff, ',');
                    cars->vehicletype_ =
                        VehicleModelInfo::findVehicleType(buff);

                    getline(strstream, cars->handling_, ',');
                    getline(strstream, cars->vehiclename_, ',');
                    getline(strstream, buff, ',');
                    cars->vehicleclass_ =
                        VehicleModelInfo::findVehicleClass(buff);

                    getline(strstream, buff, ',');
                    cars->frequency_ = std::atoi(buff.c_str());

                    getline(strstream, buff, ',');
                    cars->level_ = std::atoi(buff.c_str());

                    getline(strstream, buff, ',');
                    cars->componentrules_ = std::atoi(buff.c_str());

                    switch (cars->vehicletype_) {
                        case VehicleModelInfo::CAR:
                            getline(strstream, buff, ',');
                            cars->wheelmodel_ = std::atoi(buff.c_str());
                            getline(strstream, buff, ',');
                            cars->wheelscale_ = std::atof(buff.c_str());
                            break;
                        case VehicleModelInfo::PLANE:
                            /// @todo load LOD
                            getline(strstream, buff, ',');
                            // cars->planeLOD_ = std::atoi(buff.c_str());
                            break;
                        default:
                            break;
                    }

                    objects.emplace(cars->id(), std::move(cars));
                    break;
                }
                case PEDS: {
                    auto peds = std::make_unique<PedModelInfo>();

                    getline(strstream, buff, ',');
                    peds->setModelID(std::atoi(buff.c_str()));

                    getline(strstream, peds->name, ',');
                    getline(strstream, peds->textureslot, ',');

                    getline(strstream, buff, ',');
                    peds->pedtype_ = PedModelInfo::findPedType(buff);

                    std::string behaviour;
                    getline(strstream, behaviour, ',');
                    peds->statindex_ = find_stat_id(behaviour);
                    getline(strstream, peds->animgroup_, ',');

                    getline(strstream, buff, ',');
                    peds->carsmask_ = std::atoi(buff.c_str());

                    objects.emplace(peds->id(), std::move(peds));
                    break;
                }
                case PATH: {
                    PathData path;

                    std::string type;
                    getline(strstream, type, ',');
                    if (type == "ped") {
                        path.type = PathData::PATH_PED;
                    } else if (type == "car") {
                        path.type = PathData::PATH_CAR;
                    }

                    std::string id;
                    getline(strstream, id, ',');
                    path.ID = atoi(id.c_str());

                    getline(strstream, path.modelName);

                    std::string linebuff, buff;
                    for (size_t p = 0; p < 12; ++p) {
                        PathNode node;

                        getline(str, linebuff);
                        std::stringstream buffstream(linebuff);

                        getline(buffstream, buff, ',');
                        switch (atoi(buff.c_str())) {
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

                        if (node.type == PathNode::EMPTY) {
                            continue;
                        }

                        getline(buffstream, buff, ',');
                        node.next = atoi(buff.c_str());

                        getline(buffstream, buff, ',');  // "Always 0"

                        getline(buffstream, buff, ',');
                        node.position.x = atof(buff.c_str()) * 1 / 16.f;

                        getline(buffstream, buff, ',');
                        node.position.y = atof(buff.c_str()) * 1 / 16.f;

                        getline(buffstream, buff, ',');
                        node.position.z = atof(buff.c_str()) * 1 / 16.f;

                        getline(buffstream, buff, ',');
                        node.size = atof(buff.c_str()) * 1 / 16.f;

                        getline(buffstream, buff, ',');
                        node.other_thing = atoi(buff.c_str());

                        getline(buffstream, buff, ',');
                        node.other_thing2 = atoi(buff.c_str());

                        path.nodes.push_back(node);
                    }

                    auto &object = objects[path.ID];
                    auto simple = dynamic_cast<SimpleModelInfo *>(object.get());
                    simple->paths.push_back(path);

                    break;
                }
                case HIER: {
                    auto hier = std::make_unique<ClumpModelInfo>();

                    getline(strstream, buff, ',');
                    hier->setModelID(std::atoi(buff.c_str()));

                    getline(strstream, hier->name, ',');
                    getline(strstream, hier->textureslot, ',');

                    objects.emplace(hier->id(), std::move(hier));
                    break;
                }
            }
        }
    }

    return true;
}
