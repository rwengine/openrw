#include <loaders/LoaderIPL.hpp>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <sstream>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "data/InstanceData.hpp"
#include "data/ZoneData.hpp"
#include "LoaderIPL.hpp"


#include <rw/casts.hpp>

enum SectionTypes { INST, PICK, CULL, ZONE, NONE };

bool LoaderIPL::load(const std::string& filename) {
    std::ifstream str(filename);
    if (!str.is_open()) return false;
    return load(str);
}

bool LoaderIPL::load(std::istream &str) {
    SectionTypes section = NONE;
    while (!str.eof()) {
        std::string line;
        getline(str, line);
        line.erase(std::find_if(line.rbegin(), line.rend(),
                                [](auto c) { return !std::isspace(c); })
                       .base(),
                   line.end());

        if (!line.empty() && line[0] == '#') {
            // nothing, just a comment
        } else if (line == "end")  // terminating a section
        {
            section = NONE;
        } else if (section == NONE)  // starting a new section
        {
            if (line == "inst") {
                section = INST;
            }
            if (line == "pick") {
                section = PICK;
            }
            if (line == "cull") {
                section = CULL;
            }
            if (line == "zone") {
                section = ZONE;
            }
        } else  // regular entry
        {
            if (section == INST) {
                std::string id;
                std::string model;
                std::string posX, posY, posZ;
                std::string scaleX, scaleY, scaleZ;
                std::string rotX, rotY, rotZ, rotW;

                std::stringstream strstream(line);

                // read all the contents of the line
                getline(strstream, id, ',');
                getline(strstream, model, ',');
                getline(strstream, posX, ',');
                getline(strstream, posY, ',');
                getline(strstream, posZ, ',');
                getline(strstream, scaleX, ',');
                getline(strstream, scaleY, ',');
                getline(strstream, scaleZ, ',');
                getline(strstream, rotX, ',');
                getline(strstream, rotY, ',');
                getline(strstream, rotZ, ',');
                getline(strstream, rotW, ',');

                auto instance = std::make_shared<InstanceData>(
                    lexical_cast<int>(id),  // ID
                    model.substr(1, model.size() - 1),
                    glm::vec3(lexical_cast<float>(posX), lexical_cast<float>(posY),
                              lexical_cast<float>(posZ)),
                    glm::vec3(lexical_cast<float>(scaleX), lexical_cast<float>(scaleY),
                              lexical_cast<float>(scaleZ)),
                    glm::normalize(
                        glm::quat(-lexical_cast<float>(rotW), lexical_cast<float>(rotX),
                                  lexical_cast<float>(rotY), lexical_cast<float>(rotZ))));

                m_instances.push_back(instance);
            } else if (section == ZONE) {
                ZoneData zone;

                std::stringstream strstream(line);

                std::string value;

                getline(strstream, value, ',');
                zone.name = value;

                getline(strstream, value, ',');
                zone.type = lexical_cast<int>(value);

                getline(strstream, value, ',');
                zone.min.x = lexical_cast<float>(value);
                getline(strstream, value, ',');
                zone.min.y = lexical_cast<float>(value);
                getline(strstream, value, ',');
                zone.min.z = lexical_cast<float>(value);

                getline(strstream, value, ',');
                zone.max.x = lexical_cast<float>(value);
                getline(strstream, value, ',');
                zone.max.y = lexical_cast<float>(value);
                getline(strstream, value, ',');
                zone.max.z = lexical_cast<float>(value);

                getline(strstream, value, ',');
                zone.island = lexical_cast<int>(value);

                for (int i = 0; i < ZONE_GANG_COUNT; i++) {
                    zone.gangCarDensityDay[i] = zone.gangCarDensityNight[i] =
                        zone.gangDensityDay[i] = zone.gangDensityNight[i] = 0;
                }

                zone.pedGroupDay = 0;
                zone.pedGroupNight = 0;

                zones.push_back(std::move(zone));
            }
        }
    }

    return true;
}

