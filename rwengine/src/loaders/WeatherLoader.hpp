#ifndef _RWENGINE_WEATHERLOADER_HPP_
#define _RWENGINE_WEATHERLOADER_HPP_

#include <data/Weather.hpp>

#include <string>

namespace WeatherLoader {
bool load(const std::string &filename, Weather& outWeather);
}

#endif
