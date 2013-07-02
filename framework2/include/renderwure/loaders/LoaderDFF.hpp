#pragma once
#ifndef _LOADERDFF_HPP_
#define _LOADERDFF_HPP_

#define GLEW_STATIC
#include <GL/glew.h>

#include "../../framework/rwbinarystream.h"

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
	
	struct Material {
		std::vector<Texture> textures;
	};
	
	struct SubGeometry {
		GLuint EBO;
		size_t material;
		std::vector<uint32_t> indices;
	};
	
	struct Geometry {
		GLuint VBO, EBO;

		RW::BSGeometryBounds geometryBounds;

		std::vector<RW::BSGeometryUV> texcoords;
		std::vector<RW::BSGeometryTriangle> triangles;
		std::vector<RW::BSTVector3> vertices;
		std::vector<RW::BSTVector3> normals;

		std::vector<Material> materials;
		std::vector<SubGeometry> subgeom;
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

#endif