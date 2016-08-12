#include "loaders/LoaderCOL.hpp"
#include <algorithm>
#include <string>
#include <fstream>

typedef glm::vec3 CollTVec3;

struct CollTBounds
{
	CollTVec3 min, max;
	CollTVec3 center;
	float radius;
};

struct CollTBoundsV1
{
	float radius;
	CollTVec3 center;
	CollTVec3 min, max;
};

struct CollTSurface
{
	uint8_t material;
	uint8_t flag;
	uint8_t brightness;
	uint8_t light;
};

struct CollTSphere
{
	CollTVec3 center;
	float radius;
	CollTSurface surface;
};

struct CollTSphereV1
{
	float radius;
	CollTVec3 center;
	CollTSurface surface;
};

struct CollTBox
{
	CollTVec3 min, max;
	CollTSurface surface;
};

struct CollTFaceGroup
{
	CollTVec3 min, max;
	uint16_t startface, endface;
};

typedef glm::vec3 CollTVertex;

struct CollTFace
{
    uint16_t a, b, c;
    uint8_t material;
    uint8_t light;
};

struct CollTFaceV1
{
	uint32_t a, b, c;
	CollTSurface surface;
};

struct CollTFaceTriangle
{
    uint32_t a, b, c;
};

struct CollTFaceData
{
    uint8_t material;
    uint8_t light;
};

struct CollTHeader
{
	char magic[4];
	uint32_t size;
	char name[22];
	uint16_t modelid;
	CollTBounds bounds;
};

struct CollTHeaderV2
{
	uint16_t numspheres;
	uint16_t numboxes;
	uint32_t numfaces;
	uint32_t flags;
	uint32_t offsetspheres;
	uint32_t offsetboxes;
	uint32_t offsetlines;
	uint32_t offsetverts;
	uint32_t offsetfaces;
};

struct CollTHeaderV3
{
	uint32_t numshadowfaces;
	uint32_t offsetverts;
	uint32_t offsetfaces;
};

template<class T> T readType(char* data, size_t* offset)
{
	size_t orgoff = *offset; *offset += sizeof(T);
	return *reinterpret_cast<T*>(data+orgoff);
}

bool LoaderCOL::load(char* data, const size_t size)
{
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
		
		if(version >= 2) 
		{
			head2 = readType<CollTHeaderV2>(data, &dataI);
			if(version >= 3)
			{
				head3 = readType<CollTHeaderV3>(data, &dataI);
			}
		}
		
		CollisionModel *model = new CollisionModel;
		model->version = version;
		model->center = head.bounds.center;
		model->min = head.bounds.min;
		model->max = head.bounds.max;
		model->name = head.name;
		model->modelid = head.modelid;
		
		if(version == 1)
		{
			head2.numspheres = readType<uint32_t>(data, &dataI);
			head2.offsetspheres = file_base-4;
		}
		// Read spheres
		for( size_t i = 0; i < head2.numspheres; ++i ) {
			if(version == 1) {
				auto s1 = readType<CollTSphereV1>(data, &dataI);
				
				model->spheres.push_back({ s1.center, s1.radius });
			}
			else {
				auto s2 = readType<CollTSphere>(data, &dataI);
				
				model->spheres.push_back({ s2.center, s2.radius });
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
			auto b1 = readType<CollTBox>(data, &dataI);
			model->boxes.push_back({b1.min, b1.max});
		}

		if(version == 1)
		{
			uint32_t numverts = readType<uint32_t>(data, &dataI);
			head2.offsetverts = dataI;
			// Skip the vertex data for now, since it's accessed retroactivly.
			dataI += numverts * sizeof(CollTVertex);
		}
		else {
			// TODO support version 2 & 3.
		}

		if(version == 1) 
		{
			head2.numfaces = readType<uint32_t>(data, &dataI);
		}
		
		size_t maxvert = 0;
		model->indices.reserve(head2.numfaces * 3);
		for( size_t f = 0; f < head2.numfaces; ++f) {
			// todo: Support 2-3
			CollTFaceV1 face = readType<CollTFaceV1>(data, &dataI);
			size_t maxv = std::max(face.a, std::max(face.b, face.c));
			maxvert = std::max( maxvert, maxv );
			model->indices.push_back(face.a);
			model->indices.push_back(face.b);
			model->indices.push_back(face.c);
		}

		// Load up to maxvert vertices.
		model->vertices.reserve(maxvert+1);
		for( size_t v = 0, vertI = head2.offsetverts; v < maxvert+1; ++v ) {
			CollTVertex vert = readType<CollTVertex>(data, &vertI);
			// CollTVertex is vec3
			model->vertices.push_back(vert);
		}

		dataI += sizeof(CollTFace) * head2.numfaces;
		
		instances.push_back(std::move(std::unique_ptr<CollisionModel>(model)));
		
		dataI = file_base + head.size + sizeof(char) * 8;
	}
	
	return true;
}

bool LoaderCOL::load(const std::string& file)
{
	std::ifstream dfile(file.c_str(), std::ios_base::binary);
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
