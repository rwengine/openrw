#pragma once
#ifndef _WEATHERLOADER_HPP_
#define _WEATHERLOADER_HPP_

#include <renderwure/engine/GTATypes.hpp>

#include <string>
#include <sstream>
#include <vector>

class WeatherLoader
{
public:
	struct WeatherData {
		GTATypes::RGB ambientColor;
		GTATypes::RGB directLightColor;
		GTATypes::RGB skyTopColor;
		GTATypes::RGB skyBottomColor;
		GTATypes::RGB sunCoreColor;
		GTATypes::RGB sunCoronaColor;
		float sunCoreSize;
		float sunCoronaSize;
		float sunBrightness;
		int32_t shadowIntensity;
		int32_t lightShading;
		int32_t poleShading;
		float farClipping;
		float fogStart;
		float amountGroundLight;
		GTATypes::RGB lowCloudColor;
		GTATypes::RGB topCloudColor;
		GTATypes::RGB bottomCloudColor;
		uint8_t unknown[4];
	};

	bool load(const std::string &filename);

	std::vector<WeatherData> weather;

private:
	GTATypes::RGB readRGB(std::stringstream &ss);
};

#endif