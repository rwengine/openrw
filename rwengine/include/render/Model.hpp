#pragma once
#ifndef _MODEL_HPP_
#define _MODEL_HPP_
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <memory>
#include <GL/glew.h>

#include <loaders/rwbinarystream.h>

/** 
	* Frame
	*   => Atomic 
	*     => Geometry
	*   - defaultRotation
	*   - defaultTranslation 
	*   
	*   + setTransform(mat)
	*   + resetTransform()
	*/

class ModelFrame {
	glm::mat3 defaultRotation;
	glm::vec3 defaultTranslation;
	glm::mat4 matrix;
	ModelFrame* parentFrame;
	std::string name;
	std::vector<size_t> geometries;
	std::vector<ModelFrame*> childs;
public:
	
	ModelFrame(ModelFrame* parent, glm::mat3 dR, glm::vec3 dT);
	~ModelFrame();

	void reset();
	void setTransform(const glm::mat4& m);

	void setName(const std::string& fname) 
		{ name = fname; }

	void addGeometry(size_t idx);

	glm::vec3 getDefaultTranslation() const
		{ return defaultTranslation; }

	glm::mat3 getDefaultRotation() const 
		{ return defaultRotation; }

	glm::mat4 getMatrix() const 
		{ return (parentFrame? parentFrame->getMatrix() : glm::mat4()) * matrix; }

	const std::vector<ModelFrame*>& getChildren() const
		{ return childs; }

	const std::string& getName() const 
		{ return name; }

	const std::vector<size_t>& getGeometries() const
		{ return geometries; }
};

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
	
	std::vector<ModelFrame*> frames;
	
	std::vector<std::string> frameNames;

	std::vector<std::shared_ptr<Geometry>> geometries;
	std::vector<Atomic> atomics;

	int32_t rootFrameIdx;
};

#endif