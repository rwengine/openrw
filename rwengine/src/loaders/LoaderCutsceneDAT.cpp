#include "loaders/LoaderCutsceneDAT.hpp"

#include <algorithm>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <boost/algorithm/string.hpp>

#include <rw/defines.hpp>

#include "data/CutsceneData.hpp"
#include "platform/FileHandle.hpp"

void LoaderCutsceneDAT::load(CutsceneTracks &tracks, const FileHandle& file) {
    std::string dataStr(file->data, file->length);
    std::stringstream ss(dataStr);

    std::vector<std::string> lineParts;

    int numZooms = 0;
    ss >> numZooms;
    ss.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    for (int i = 0; i < numZooms; ++i) {
        std::string line;
        ss >> line;

        boost::split(lineParts, line, boost::is_any_of(","));

        float t = stof(lineParts[0]);
        float z = stof(lineParts[1]);

        tracks.zoom[t] = z;
        tracks.duration = std::max(t, tracks.duration);
    }

    ss.ignore(std::numeric_limits<std::streamsize>::max(), ';');

    int numRotations = 0;
    ss >> numRotations;
    ss.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    for (int i = 0; i < numRotations; ++i) {
        std::string line;
        ss >> line;

        boost::split(lineParts, line, boost::is_any_of(","));

        float t = stof(lineParts[0]);
        float r = stof(lineParts[1]);

        tracks.rotation[t] = r;
        tracks.duration = std::max(t, tracks.duration);
    }

    ss.ignore(std::numeric_limits<std::streamsize>::max(), ';');

    int numPositions = 0;
    ss >> numPositions;
    ss.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    for (int i = 0; i < numPositions; ++i) {
        std::string line;
        ss >> line;

        boost::split(lineParts, line, boost::is_any_of(","));

        float t = stof(lineParts[0]);
        glm::vec3 p;
        p.x = stof(lineParts[1]);
        p.y = stof(lineParts[2]);
        p.z = stof(lineParts[3]);

        tracks.position[t] = p;
        tracks.duration = std::max(t, tracks.duration);
    }

    ss.ignore(std::numeric_limits<std::streamsize>::max(), ';');

    int numTargets = 0;
    ss >> numTargets;
    ss.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    for (int i = 0; i < numTargets; ++i) {
        std::string line;
        ss >> line;

        boost::split(lineParts, line, boost::is_any_of(","));

        float t = stof(lineParts[0]);
        glm::vec3 p;
        p.x = stof(lineParts[1]);
        p.y = stof(lineParts[2]);
        p.z = stof(lineParts[3]);

        tracks.target[t] = p;
        tracks.duration = std::max(t, tracks.duration);
    }

    RW_CHECK(ss.eof() || ss.good(), "Loading CutsceneDAT file failed");
}
