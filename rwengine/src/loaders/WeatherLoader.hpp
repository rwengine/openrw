#pragma once
#ifndef _WEATHERLOADER_HPP_
#define _WEATHERLOADER_HPP_

#include <rw/types.hpp>

#include <string>
#include <sstream>
#include <vector>

class WeatherLoader
{
public:
  struct WeatherData {
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

  enum WeatherCondition { Sunny = 0, Cloudy = 24, Rainy = 48, Foggy = 72 };

  bool load(const std::string &filename);

  std::vector<WeatherData> weather;

  /**
   * @brief getWeatherData returns interpolated Weather data for the time of day.
   * @param cond weather condition
   * @param tod float time of day
   * @return Correctly interpolated values.
   */
  WeatherData getWeatherData(WeatherCondition cond, float tod);

private:
  RWTypes::RGB readRGB(std::stringstream &ss);
};

#endif
