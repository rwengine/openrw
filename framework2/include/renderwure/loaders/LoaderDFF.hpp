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
		uint32_t colour;
		
		float diffuseIntensity;
		float ambientIntensity;
	};
	
	struct SubGeometry {
		GLuint EBO;
		size_t material;
		std::vector<uint32_t> indices;
	};
	
	struct Geometry {
		GLuint VBO, EBO;

		RW::BSGeometryBounds geometryBounds;
		
		uint32_t clumpNum;

		uint32_t flags;
		
		std::vector<RW::BSGeometryUV> texcoords;
		std::vector<RW::BSGeometryTriangle> triangles;
		std::vector<glm::vec4> colours;
		std::vector<RW::BSTVector3> vertices;
		std::vector<RW::BSTVector3> normals;

		std::vector<Material> materials;
		std::vector<SubGeometry> subgeom;
	};
	
	struct Atomic {
		uint32_t frame;
		uint32_t geometry;
	};

    struct Frame {
        glm::mat4 matrix;
        glm::mat3 defaultRotation;
        glm::vec3 defaultTranslation;
        int32_t parentFrameIndex;
    };
	
	std::vector<std::string> frameNames;

	std::vector<Geometry> geometries;
	std::vector<Atomic> atomics;
    std::vector<Frame> frames;

    glm::mat4 getFrameMatrix(int32_t frameIndex)
    {
        Frame& frame = frames[frameIndex];
        if( frame.parentFrameIndex != -1 ) {
            return getFrameMatrix(frame.parentFrameIndex) * frame.matrix;
        }
        else {
            return frame.matrix;
        }
    }
};

class LoaderDFF
{
private:
	template<class T> T readStructure(char *data, size_t &dataI);
	RW::BSSectionHeader readHeader(char *data, size_t &dataI);

public:
    Model* loadFromMemory(char *data);
};

#endif
