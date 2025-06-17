#ifndef _RWENGINE_WEATHER_HPP_
#define _RWENGINE_WEATHER_HPP_

#include <glm/vec3.hpp>

#include <rw/types.hpp>

#include <cstdint>
#include <vector>


enum class WeatherCondition { Sunny = 0, Cloudy = 1, Rainy = 2, Foggy = 3 };

class Weather {
public:
    struct Entry {
        glm::vec3 ambientColor{};
        glm::vec3 directLightColor{};
        glm::vec3 skyTopColor{};
        glm::vec3 skyBottomColor{};
        glm::vec3 sunCoreColor{};
        glm::vec3 sunCoronaColor{};
        float sunCoreSize;
        float sunCoronaSize;
        float sunBrightness;
        int32_t shadowIntensity;
        int32_t lightShading;
        int32_t poleShading;
        float farClipping;
        float fogStart;
        float amountGroundLight;
        glm::vec3 lowCloudColor{};
        glm::vec3 topCloudColor{};
        glm::vec3 bottomCloudColor{};
        uint8_t unknown[4];
    };

    Entry interpolate(WeatherCondition lastWeather,
                      WeatherCondition nextWeather,
                      float a, float tod);

    std::vector<Entry> entries;

    // Taken from: https://www.gtamodding.com/wiki/Time_cycle#Weather_lists
    // TODO: This weather list applies only for GTA III
    const uint16_t WeatherList[64] = {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 1, 0,
            0, 0, 1, 3, 3, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2,
            2, 1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 0, 0, 0, 2, 1
    };
};

#endif
