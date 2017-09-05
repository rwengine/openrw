#include <algorithm>
#include <loaders/LoaderCutsceneDAT.hpp>
#include <sstream>

void LoaderCutsceneDAT::load(CutsceneTracks &tracks, FileHandle file) {
    std::string dataStr(file->data, file->length);
    std::stringstream ss(dataStr);

    int numZooms = 0;
    ss >> numZooms;
    ss.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    for (int i = 0; i < numZooms; ++i) {
        float t = 0.f;
        float z = 0.f;
        ss >> t;
        ss.ignore(2, ',');
        ss >> z;
        ss.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        tracks.zoom[t] = z;
        tracks.duration = std::max(t, tracks.duration);
    }

    ss.ignore(std::numeric_limits<std::streamsize>::max(), ';');

    int numRotations = 0;
    ss >> numRotations;
    ss.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    for (int i = 0; i < numRotations; ++i) {
        float t = 0.f;
        float r = 0.f;
        ss >> t;
        ss.ignore(2, ',');
        ss >> r;
        ss.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        tracks.rotation[t] = r;
        tracks.duration = std::max(t, tracks.duration);
    }

    ss.ignore(std::numeric_limits<std::streamsize>::max(), ';');

    int numPositions = 0;
    ss >> numPositions;
    ss.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    for (int i = 0; i < numPositions; ++i) {
        float t = 0.f;
        glm::vec3 p;
        ss >> t;
        ss.ignore(2, ',');
        ss >> p.x;
        ss.ignore(1, ',');
        ss >> p.y;
        ss.ignore(1, ',');
        ss >> p.z;
        ss.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        tracks.position[t] = p;
        tracks.duration = std::max(t, tracks.duration);
    }

    ss.ignore(std::numeric_limits<std::streamsize>::max(), ';');

    int numTargets = 0;
    ss >> numTargets;
    ss.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    for (int i = 0; i < numTargets; ++i) {
        float t = 0.f;
        glm::vec3 p;
        ss >> t;
        ss.ignore(2, ',');
        ss >> p.x;
        ss.ignore(1, ',');
        ss >> p.y;
        ss.ignore(1, ',');
        ss >> p.z;
        ss.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        tracks.target[t] = p;
        tracks.duration = std::max(t, tracks.duration);
    }
}
