#include "Weather.hpp"


namespace {
Weather::Entry interpolateWeather(const Weather::Entry& a,
                                  const Weather::Entry& b,
                                  float t) {
#define MIXPROP(prop) glm::mix(a.prop, b.prop, t)
    return {
            MIXPROP(ambientColor),
            MIXPROP(directLightColor),
            MIXPROP(skyTopColor),
            MIXPROP(skyBottomColor),
            MIXPROP(sunCoreColor),
            MIXPROP(sunCoronaColor),
            MIXPROP(sunCoreSize),
            MIXPROP(sunCoronaSize),
            MIXPROP(sunBrightness),
            MIXPROP(shadowIntensity),
            MIXPROP(lightShading),
            MIXPROP(poleShading),
            MIXPROP(farClipping),
            MIXPROP(fogStart),
            MIXPROP(amountGroundLight),
            MIXPROP(lowCloudColor),
            MIXPROP(topCloudColor),
            MIXPROP(bottomCloudColor),
            {}
    };
#undef MIXPROP
}
}

Weather::Entry Weather::interpolate(WeatherCondition prev,
                                    WeatherCondition next,
                                    float a, float tod) {
    const float t = tod - std::floor(tod);
    const auto nI = size_t(next) * 24;
    const auto pI = size_t(prev) * 24;
    const auto hour = size_t(tod);

    RW_ASSERT(nI < entries.size());
    const auto& x = entries[nI + hour];
    const auto& y = entries[nI + (hour + 1) % 24];

    const auto& nextWeather = interpolateWeather(x, y, t);

    if (a >= 1.0f) {
        return nextWeather;
    }

    RW_ASSERT(pI < entries.size());
    const auto& z = entries[pI + hour];
    const auto& w = entries[pI + (hour + 1) % 24];

    const auto& prevWeather = interpolateWeather(z, w, t);

    return interpolateWeather(prevWeather, nextWeather, a);
}
