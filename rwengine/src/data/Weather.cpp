#include "Weather.hpp"

#define MIXPROP(prop) data.prop = glm::mix(x.prop, y.prop, a)

Weather::Entry Weather::getWeatherData(WeatherCondition cond, float tod) {
    const auto i = size_t(cond) * 24;
    RW_ASSERT(i < entries.size());

    size_t hour = std::floor(tod);
    const auto& x = entries[i + hour];
    const auto& y = entries[i + (hour + 1) % 24];
    const float a = tod - std::floor(tod);

    Entry data;
    MIXPROP(ambientColor);
    MIXPROP(directLightColor);
    MIXPROP(skyTopColor);
    MIXPROP(skyBottomColor);
    MIXPROP(sunCoreColor);
    MIXPROP(sunCoreSize);
    MIXPROP(sunCoronaSize);
    MIXPROP(sunBrightness);
    MIXPROP(shadowIntensity);
    MIXPROP(lightShading);
    MIXPROP(poleShading);
    MIXPROP(farClipping);
    MIXPROP(fogStart);
    MIXPROP(amountGroundLight);
    MIXPROP(lowCloudColor);
    MIXPROP(topCloudColor);
    MIXPROP(bottomCloudColor);

    return data;
}
