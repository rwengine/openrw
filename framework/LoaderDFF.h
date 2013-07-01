#pragma once

#define GLEW_STATIC
#include <GL/glew.h>

#include "rwbinarystream.h"

#include <vector>
#include <string>
#include <memory>

class Model
{
public:
	RW::BSClump clump;

	struct Texture {
		std::string name;
		std::string alphaName;
	};
	struct Geometry {
		GLuint VBO, EBO;

		RW::BSGeometryBounds geometryBounds;

		std::vector<RW::BSGeometryUV> texcoords;
		std::vector<RW::BSGeometryTriangle> triangles;
		std::vector<RW::BSTVector3> vertices;
		std::vector<RW::BSTVector3> normals;

		std::vector<Texture> textures;
	};

	std::vector<Geometry> geometries;
};

class LoaderDFF
{
private:
	template<class T> T readStructure(char *data, size_t &dataI);
	RW::BSSectionHeader readHeader(char *data, size_t &dataI);

public:
	std::unique_ptr<Model> loadFromMemory(char *data);
};
