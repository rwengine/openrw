#ifndef _RWENGINE_WEATHER_HPP_
#define _RWENGINE_WEATHER_HPP_

#include <rw/types.hpp>

#include <cstdint>
#include <vector>

enum class WeatherCondition { Sunny = 0, Cloudy = 1, Rainy = 2, Foggy = 3 };

class Weather {
public:
    struct Entry {
        RWTypes::RGB ambientColor;
        RWTypes::RGB directLightColor;
        RWTypes::RGB skyTopColor;
        RWTypes::RGB skyBottomColor;
        RWTypes::RGB sunCoreColor;
        RWTypes::RGB sunCoronaColor;
        float sunCoreSize;
        float sunCoronaSize;
        float sunBrightness;
        int32_t shadowIntensity;
        int32_t lightShading;
        int32_t poleShading;
        float farClipping;
        float fogStart;
        float amountGroundLight;
        RWTypes::RGB lowCloudColor;
        RWTypes::RGB topCloudColor;
        RWTypes::RGB bottomCloudColor;
        uint8_t unknown[4];
    };

    /**
     * @brief getWeatherData returns interpolated Weather data for the time of
     * day.
     * @param cond weather condition
     * @param tod float time of day
     * @return Correctly interpolated values.
     */
    Entry getWeatherData(WeatherCondition condition, float tod);

    std::vector<Entry> entries;
};

#endif
