#include <renderwure/loaders/WeatherLoader.hpp>

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

GTAData::RGB WeatherLoader::readRGB(std::stringstream &ss)
{
	GTAData::RGB color;
	std::string r, g, b;

	std::getline(ss, r, ' ');
	std::getline(ss, g, ' ');
	std::getline(ss, b, ' ');

	color.r = atoi(r.c_str());
	color.b = atoi(b.c_str());
	color.g = atoi(g.c_str());

	return color;
}
