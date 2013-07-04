#pragma once
#ifndef _WEATHERLOADER_HPP_
#define _WEATHERLOADER_HPP_

#include <renderwure/engine/GTAData.hpp>

#include <string>
#include <sstream>

class WeatherLoader
{
public:
	struct WeatherData {
		GTAData::RGB ambientColor;
		GTAData::RGB directLightColor;
		GTAData::RGB skyTopColor;
		GTAData::RGB skyBottomColor;
		GTAData::RGB sunCoreColor;
		GTAData::RGB sunCoronaColor;
		float sunCoreSize;
		float sunCoronaSize;
		float sunBrightness;
		int32_t shadowIntensity;
		int32_t lightShading;
		int32_t poleShading;
		float farClipping;
		float fogStart;
		float amountGroundLight;
		GTAData::RGB lowCloudColor;
		GTAData::RGB topCloudColor;
		GTAData::RGB bottomCloudColor;
		uint8_t unknown[4];
	};

	bool load(const std::string &filename);

	std::vector<WeatherData> weather;

private:
	GTAData::RGB readRGB(std::stringstream &ss);
};

#endif