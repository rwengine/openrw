#pragma once
#ifndef _MODEL_HPP_
#define _MODEL_HPP_
#include <vector>
#include <string>
#include <memory>
#include <glm/glm.hpp>
#include <GL/glew.h>

#include <loaders/rwbinarystream.h>

class Model
{
public:
    enum FaceType {
        Triangles = 0,
        TriangleStrip = 1
    };

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
		GLuint start = 0;
        size_t material;
		uint32_t* indices;
		size_t numIndices;
	};
	
	struct Geometry {
		GLuint VBO, EBO;
		
		RW::BSGeometryBounds geometryBounds;
		
		uint32_t clumpNum;

        FaceType facetype;

		uint32_t flags;
		
		size_t offsVert;
		std::vector<glm::vec3> vertices;
		size_t offsNormals;
		std::vector<glm::vec3> normals;
		size_t offsTexCoords;
		std::vector<glm::vec2> texcoords;
		size_t offsColours;
		std::vector<glm::vec4> colours;
		
		size_t indicesCount;

		std::vector<Material> materials;
		std::vector<SubGeometry> subgeom;
		
		Geometry();
		~Geometry();
		
		void setVertexData(const std::vector<glm::vec3>& vertices) {
			this->vertices = vertices;
		}
		void setColours(const std::vector<glm::vec4>& colours) {
			this->colours = colours;
		}
		void setTexCoords(const std::vector<glm::vec2>& coords) {
			this->texcoords = coords;
		}
		void setNormals(const std::vector<glm::vec3>& normals) {
			this->normals = normals;
		}
		void generateNormals();
		
		void buildBuffers();
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

	std::vector<std::shared_ptr<Geometry>> geometries;
	std::vector<Atomic> atomics;
    std::vector<Frame> frames;

	int32_t rootFrameIdx;
	
    glm::mat4 getFrameMatrix(int32_t frameIndex)
    {
        Frame& frame = frames[frameIndex];
        if( frame.parentFrameIndex != -1 ) {
            return frame.matrix * getFrameMatrix(frame.parentFrameIndex);
        }
        else {
            return frame.matrix;
        }
    }
};

#endif