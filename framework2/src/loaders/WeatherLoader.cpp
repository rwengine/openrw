#include <loaders/WeatherLoader.hpp>

#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>

bool WeatherLoader::load(const std::string &filename)
{
	std::ifstream fstream(filename.c_str());

	if ( ! fstream.is_open())
		return false;
	
	std::string line;
	while (std::getline(fstream, line)) {
		if (line[0] == '/') // Comment line
			continue;

		WeatherData weather;

		// Convert tabs into spaces
		std::replace(line.begin(), line.end(), '\t', ' ');
		// Remove all duplicate whitespace
		line.erase(std::unique(line.begin(), line.end(), [](char l, char r) { return l == r && std::isspace(l); }), line.end());

		std::stringstream ss(line);
		std::string tmpstr;

		weather.ambientColor = readRGB(ss);
		weather.directLightColor = readRGB(ss);
		weather.skyTopColor = readRGB(ss);
		weather.skyBottomColor = readRGB(ss);
		weather.sunCoreColor = readRGB(ss);
		weather.sunCoronaColor = readRGB(ss);

		std::getline(ss, tmpstr, ' '); weather.sunCoreSize = atof(tmpstr.c_str());
		std::getline(ss, tmpstr, ' '); weather.sunCoronaSize = atof(tmpstr.c_str());
		std::getline(ss, tmpstr, ' '); weather.sunBrightness = atof(tmpstr.c_str());
		std::getline(ss, tmpstr, ' '); weather.shadowIntensity = atoi(tmpstr.c_str());
		std::getline(ss, tmpstr, ' '); weather.lightShading = atoi(tmpstr.c_str());
		std::getline(ss, tmpstr, ' '); weather.poleShading = atoi(tmpstr.c_str());
		std::getline(ss, tmpstr, ' '); weather.farClipping = atof(tmpstr.c_str());
		std::getline(ss, tmpstr, ' '); weather.fogStart = atof(tmpstr.c_str());
		std::getline(ss, tmpstr, ' '); weather.amountGroundLight = atof(tmpstr.c_str());

		weather.lowCloudColor = readRGB(ss);
		weather.topCloudColor = readRGB(ss);
		weather.bottomCloudColor = readRGB(ss);

		for (size_t i = 0; i < 4; i++) {
			std::getline(ss, tmpstr, ' ');
			weather.unknown[i] = atoi(tmpstr.c_str());
		}

		this->weather.push_back(weather);
	}

	return true;
}

GTATypes::RGB mix(GTATypes::RGB x, GTATypes::RGB y, float a)
{
	GTATypes::RGB n;
	n.r = x.r * (1.f - a) + y.r * a;
	n.g = x.g * (1.f - a) + y.g * a;
	n.b = x.b * (1.f - a) + y.b * a;
	return n;
}

int32_t mixint(int32_t x, int32_t y, float a)
{
	return x * (1.f - a) + y * a;
}

#define MIXPROP(prop) data.prop = mix(x.prop, y.prop, a)
#define MIXPROP2(prop) data.prop = glm::mix(x.prop, y.prop, a)
#define MIXPROP_INT(prop) data.prop = mixint(x.prop, y.prop, a)

WeatherLoader::WeatherData WeatherLoader::getWeatherData(WeatherCondition cond, float tod)
{
	size_t hour = floor(tod);
	const WeatherData& x = weather[static_cast<size_t>(cond)+hour];
	const WeatherData& y = weather[static_cast<size_t>(cond)+(hour+1)%24];
	const float a = tod - floor(tod);

	WeatherData data;
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

GTATypes::RGB WeatherLoader::readRGB(std::stringstream &ss)
{
	GTATypes::RGB color;
	std::string r, g, b;

	std::getline(ss, r, ' ');
	std::getline(ss, g, ' ');
	std::getline(ss, b, ' ');

	color.r = atoi(r.c_str());
	color.b = atoi(b.c_str());
	color.g = atoi(g.c_str());

	return color;
}
