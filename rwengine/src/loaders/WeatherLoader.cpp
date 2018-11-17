#include <loaders/WeatherLoader.hpp>

#include <algorithm>
#include <fstream>
#include <sstream>

namespace {
glm::vec3 readRGB(std::stringstream& ss) {
    int r, g, b;
    ss >> r >> g >> b;
    return {r / 255.f, g / 255.f, b / 255.f};
}
}

bool WeatherLoader::load(const std::string& filename, Weather& outWeather) {
    std::ifstream fstream(filename.c_str());

    if (!fstream.is_open())
        return false;

    std::string line;
    while (std::getline(fstream, line)) {
        if (line[0] == '/')  // Comment line
            continue;

        Weather::Entry weather;
        std::stringstream ss(line);

        weather.ambientColor = readRGB(ss);
        weather.directLightColor = readRGB(ss);
        weather.skyTopColor = readRGB(ss);
        weather.skyBottomColor = readRGB(ss);
        weather.sunCoreColor = readRGB(ss);
        weather.sunCoronaColor = readRGB(ss);

        ss >> weather.sunCoreSize;
        ss >> weather.sunCoronaSize;
        ss >> weather.sunBrightness;
        ss >> weather.shadowIntensity;
        ss >> weather.lightShading;
        ss >> weather.poleShading;
        ss >> weather.farClipping;
        ss >> weather.fogStart;
        ss >> weather.amountGroundLight;

        weather.lowCloudColor = readRGB(ss);
        weather.topCloudColor = readRGB(ss);
        weather.bottomCloudColor = readRGB(ss);

        int d;
        for (auto& i : weather.unknown) {
            ss >> d;
            i = d;
        }

        outWeather.entries.push_back(weather);
    }

    return true;
}
