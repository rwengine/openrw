#include "loaders/LoaderCutsceneDAT.hpp"

#include <algorithm>
#include <limits>
#include <sstream>
#include <string>

#include <glm/glm.hpp>

#include <rw/debug.hpp>

#include "data/CutsceneData.hpp"
#include "platform/FileHandle.hpp"

void LoaderCutsceneDAT::load(CutsceneTracks &tracks, const FileContentsInfo& file) {
    std::string dataStr(file.data.get(), file.length);
    std::stringstream ss(dataStr);

    int numZooms = 0;
    ss >> numZooms;
    ss.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    for (int i = 0; i < numZooms; ++i) {
        std::string st, sz;
        std::getline(ss, st, ',');
        std::getline(ss, sz, ',');

        float t = std::stof(st);

        tracks.zoom[t] = std::stof(sz);
        tracks.duration = std::max(t, tracks.duration);

        ss.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    ss.ignore(std::numeric_limits<std::streamsize>::max(), ';');

    int numRotations = 0;
    ss >> numRotations;
    ss.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    for (int i = 0; i < numRotations; ++i) {
        std::string st, sr;
        std::getline(ss, st, ',');
        std::getline(ss, sr, ',');

        float t = std::stof(st);

        tracks.rotation[t] = std::stof(sr);
        tracks.duration = std::max(t, tracks.duration);

        ss.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    ss.ignore(std::numeric_limits<std::streamsize>::max(), ';');

    int numPositions = 0;
    ss >> numPositions;
    ss.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    for (int i = 0; i < numPositions; ++i) {
        std::string st, sx, sy, sz;
        std::getline(ss, st, ',');
        std::getline(ss, sx, ',');
        std::getline(ss, sy, ',');
        std::getline(ss, sz, ',');

        float t = std::stof(st);
        glm::vec3 p{std::stof(sx), std::stof(sy), std::stof(sz)};

        tracks.position[t] = p;
        tracks.duration = std::max(t, tracks.duration);

        ss.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    ss.ignore(std::numeric_limits<std::streamsize>::max(), ';');

    int numTargets = 0;
    ss >> numTargets;
    ss.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    for (int i = 0; i < numTargets; ++i) {
        std::string st, sx, sy, sz;
        std::getline(ss, st, ',');
        std::getline(ss, sx, ',');
        std::getline(ss, sy, ',');
        std::getline(ss, sz, ',');

        float t = std::stof(st);
        glm::vec3 p{std::stof(sx), std::stof(sy), std::stof(sz)};

        tracks.target[t] = p;
        tracks.duration = std::max(t, tracks.duration);

        ss.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    RW_CHECK(ss.eof() || ss.good(), "Loading CutsceneDAT file failed");
}
