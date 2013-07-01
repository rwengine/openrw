#pragma once

#include "rwbinarystream.h"

#include <vector>
#include <string>

class LoaderDFF
{
private:
	template<class T> T readStructure(char *data, size_t &dataI);
	RW::BSSectionHeader readHeader(char *data, size_t &dataI);

	RW::BSClump clump;

public:
	void loadFromMemory(char *data);

	struct Texture {
		std::string name;
		std::string alphaName;
	};
	struct Geometry {
		RW::BSGeometryBounds geometryBounds;

		std::vector<RW::BSGeometryUV> texcoords;
		std::vector<RW::BSGeometryTriangle> triangles;
		std::vector<RW::BSTVector3> vertices;
		std::vector<RW::BSTVector3> normals;

		std::vector<Texture> textures;
	};

	std::vector<Geometry> geometries;
};
