#include <renderwure/loaders/LoaderIPL.hpp>

#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>

enum SectionTypes
{
	INST,
	PICK,
	CULL,
	ZONE,
	NONE
};

/// Load the IPL data into memory
bool LoaderIPL::load(const std::string& filename)
{
	std::ifstream str(filename);

	if(!str.is_open())
		return false;

	SectionTypes section = NONE;
	while(!str.eof())
	{
		std::string line;
		getline(str, line);
		line.erase(std::find_if(line.rbegin(), line.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), line.end());
		
		if(!line.empty() && line[0] == '#')
		{
			// nothing, just a comment
		}
		else if(line == "end") // terminating a section
		{
			section = NONE;
		}
		else if(section == NONE) // starting a new section
		{
			if(line == "inst")
			{
				section = INST;
			}
			if(line == "pick")
			{
				section = PICK;
			}
			if(line == "cull")
			{
				section = CULL;
			}
			if(line == "zone")
			{
				section = ZONE;
			}
		}
		else // regular entry
		{
			if(section == INST)
			{
				std::string id;
				std::string model;
				std::string posX, posY, posZ;
				std::string scaleX, scaleY, scaleZ;
				std::string rotX, rotY, rotZ, rotW;

				std::stringstream strstream(line);
				
				// read all the contents of the line
				getline(strstream, id, ',');
				getline(strstream, model, ',');
				getline(strstream, posX, ',');
				getline(strstream, posY, ',');
				getline(strstream, posZ, ',');
				getline(strstream, scaleX, ',');
				getline(strstream, scaleY, ',');
				getline(strstream, scaleZ, ',');
				getline(strstream, rotX, ',');
				getline(strstream, rotY, ',');
				getline(strstream, rotZ, ',');
				getline(strstream, rotW, ',');
				
				std::shared_ptr<InstanceData> instance(new InstanceData{
					atoi(id.c_str()), // ID
					model.substr(1, model.size()-1), 
					glm::vec3(atof(posX.c_str()), atof(posY.c_str()), atof(posZ.c_str())),
					glm::vec3(atof(scaleX.c_str()), atof(scaleY.c_str()), atof(scaleZ.c_str())),
					glm::normalize(glm::quat(-atof(rotW.c_str()), atof(rotX.c_str()), atof(rotY.c_str()), atof(rotZ.c_str())))
				});
				
				m_instances.push_back(instance);
			}
			else if(section == ZONE) 
			{
				Zone zone;
				
				std::stringstream strstream(line);
				
				std::string value;
				
				getline(strstream, value, ',');
				zone.name = value;
				
				getline(strstream, value, ',');
				zone.type = atoi(value.c_str());
				
				getline(strstream, value, ',');
				zone.min.x = atof(value.c_str());
				getline(strstream, value, ',');
				zone.min.y = atof(value.c_str());
				getline(strstream, value, ',');
				zone.min.z = atof(value.c_str());
				
				getline(strstream, value, ',');
				zone.max.x = atof(value.c_str());
				getline(strstream, value, ',');
				zone.max.y = atof(value.c_str());
				getline(strstream, value, ',');
				zone.max.z = atof(value.c_str());
				
				getline(strstream, value, ',');
				zone.island = atoi(value.c_str());
				
				zones.push_back(zone);
			}
		}

	}

	return true;
}