#include "renderwure/loaders/LoaderCOL.hpp"
#include <string>
#include <fstream>

template<class T> T readType(char* data, size_t* offset)
{
	size_t orgoff = *offset; *offset += sizeof(T);
	return *reinterpret_cast<T*>(data+orgoff);
}

bool LoaderCOL::load(char* data, const size_t size)
{
	char* dataptr = data;
	int version = 1;
	size_t dataI = 0;
	
	while( dataI < size ) {
		size_t file_base = dataI;
		CollTHeader head = readType<CollTHeader>(data, &dataI);
		switch(head.magic[3]) {
			case 'L':
				break;
			case '2':
				version = 2;
				break;
			case '3':
				version = 3;
				break;
		}
		
		CollTHeaderV2 head2;
		CollTHeaderV3 head3;
		std::vector<CollTSphere> spheres;
		std::vector<CollTBox> boxes;
		
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
			head2.offsetspheres = file_base-4;
		}
		// Read spheres
		for( size_t i = 0; i < head2.numspheres; ++i ) {
			if(version == 1) {
				auto s1 = readType<CollTSphereV1>(data, &dataI);
				spheres.push_back({ s1.center, s1.radius, s1.surface });
			}
			else {
				spheres.push_back(readType<CollTSphere>(data, &dataI));
			}
		}
		
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
		for( size_t i = 0; i < head2.numboxes; ++i) {
			boxes.push_back(readType<CollTBox>(data, &dataI));
		}
		
		if(version == 1)
		{
			//head2.
			uint32_t numverts = readType<uint32_t>(data, &dataI);
			dataI += sizeof(CollTVertex) * numverts;
		}
		
		if(version == 1) 
		{
			head2.numfaces = readType<uint32_t>(data, &dataI);
		}
		dataI += sizeof(CollTFace) * head2.numfaces;
		
		instances.push_back({
			version,
			head,
			head2,
			head3,
			spheres,
			boxes
		});
		
		dataI = file_base + head.size + sizeof(char) * 8;
	}
	
	return true;
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
	
	auto res = load(data, length);
	
	delete[] data;
	
	return res;
}
