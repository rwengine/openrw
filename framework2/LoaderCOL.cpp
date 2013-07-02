#include "loaders/LoaderCOL.h"
#include <string>
#include <fstream>

template<class T> T readType(char* data, size_t* offset)
{
	size_t orgoff = *offset; *offset += sizeof(T);
	return *reinterpret_cast<T*>(data+orgoff);
}

bool LoaderCOL::load(const std::string& file)
{
	std::ifstream dfile(file.c_str());
	if ( ! dfile.is_open()) {
		return false;
	}

	dfile.seekg(0, std::ios_base::end);
	size_t length = dfile.tellg();
	dfile.seekg(0);
	char* data = new char[length];
	dfile.read(data, length);
	
	char* dataptr = data;
	int version = 1;
	std::string verstr(dataptr, 4);
	if(verstr == "COLL")
	{
	}
	else if(verstr == "COL2")
	{
		version = 2;
	}
	else if(verstr == "COL3")
	{
		version = 3;
	}
	
	size_t dataI = 4;
	
	auto filesize = readType<uint32_t>(data, &dataI);
	CollTHeader head = readType<CollTHeader>(data, &dataI);
	CollTHeaderV2 head2;
	CollTHeaderV3 head3;
	if(version >= 2) 
	{
		head2 = readType<CollTHeaderV2>(data, &dataI);
		if(version >= 3)
		{
			head3 = readType<CollTHeaderV3>(data, &dataI);
		}
	}
	
	if(version == 1)
	{
		head2.numspheres = readType<uint32_t>(data, &dataI);
		head2.offsetspheres = dataI-4;
	}
	// Read spheres
	dataI += sizeof(CollTSphere) * head2.numspheres;
	
	if(version == 1)
	{
		// skip unused bytes
		dataI += sizeof(uint32_t);
	}
	
	if(version == 1)
	{
		head2.numboxes = readType<uint32_t>(data, &dataI);
		head2.offsetboxes = dataI-4;
	}
	dataI += sizeof(CollTBox) * head2.numboxes;
	
	if(version == 1)
	{
		uint32_t numverts = readType<uint32_t>(data, &dataI);
		dataI += sizeof(CollTVertex) * numverts;
	}
	
	

	return true;
}
