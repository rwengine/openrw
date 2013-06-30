#include "LoaderIPL.h"

#include <fstream>
#include <string>
#include <sstream>

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
				LoaderIPLInstance instance;

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

				// convert to our structure
				instance.id = atoi(id.c_str());
				instance.model = model.substr(1, model.size()-1);
				instance.posX = atof(posX.c_str());
				instance.posY = atof(posY.c_str());
				instance.posZ = atof(posZ.c_str());
				instance.scaleX = atof(scaleX.c_str());
				instance.scaleY = atof(scaleY.c_str());
				instance.scaleZ = atof(scaleZ.c_str());
				instance.rotX = atof(rotX.c_str());
				instance.rotY = atof(rotY.c_str());
				instance.rotZ = atof(rotZ.c_str());
				instance.rotW = atof(rotW.c_str());

				/*std::cout << "id: " << instance.id << std::endl;
				std::cout << "model: " << instance.model << std::endl;
				std::cout << "posX: " << instance.posX << std::endl;
				std::cout << "posY: " << instance.posY << std::endl;
				std::cout << "posZ: " << instance.posZ << std::endl;
				std::cout << "rotW: " << instance.rotW << std::endl;*/

				m_instances.push_back(instance);
			}
		}

	}

	return true;
}