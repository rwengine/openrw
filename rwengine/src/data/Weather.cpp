#include "Weather.hpp"

RWTypes::RGB mix(RWTypes::RGB x, RWTypes::RGB y, float a) {
    RWTypes::RGB n;
    n.r = x.r * (1.f - a) + y.r * a;
    n.g = x.g * (1.f - a) + y.g * a;
    n.b = x.b * (1.f - a) + y.b * a;
    return n;
}

int32_t mixint(int32_t x, int32_t y, float a) {
    return x * (1.f - a) + y * a;
}

#define MIXPROP(prop) data.prop = mix(x.prop, y.prop, a)
#define MIXPROP2(prop) data.prop = glm::mix(x.prop, y.prop, a)
#define MIXPROP_INT(prop) data.prop = mixint(x.prop, y.prop, a)

Weather::Entry Weather::getWeatherData(WeatherCondition cond, float tod) {
    size_t hour = floor(tod);
    const auto& x = entries[static_cast<size_t>(cond) + hour];
    const auto& y = entries[static_cast<size_t>(cond) + (hour + 1) % 24];
    const float a = tod - std::floor(tod);

    Entry data;
    MIXPROP(ambientColor);
    MIXPROP(directLightColor);
    MIXPROP(skyTopColor);
    MIXPROP(skyBottomColor);
    MIXPROP(sunCoreColor);
    MIXPROP2(sunCoreSize);
    MIXPROP2(sunCoronaSize);
    MIXPROP2(sunBrightness);
    MIXPROP_INT(shadowIntensity);
    MIXPROP_INT(lightShading);
    MIXPROP_INT(poleShading);
    MIXPROP2(farClipping);
    MIXPROP2(fogStart);
    MIXPROP2(amountGroundLight);
    MIXPROP(lowCloudColor);
    MIXPROP(topCloudColor);
    MIXPROP(bottomCloudColor);

    return data;
}
