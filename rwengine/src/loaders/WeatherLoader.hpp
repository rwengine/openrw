#ifndef _RWENGINE_WEATHERLOADER_HPP_
#define _RWENGINE_WEATHERLOADER_HPP_

#include <string>

class Weather;

namespace WeatherLoader {
bool load(const std::string &filename, Weather& outWeather);
}

#endif
