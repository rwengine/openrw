#pragma once
#ifndef _MODEL_HPP_
#define _MODEL_HPP_
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <memory>
#include <GL/glew.h>

#include <loaders/rwbinarystream.h>
#include "DrawBuffer.hpp"
#include "GeometryBuffer.hpp"

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
	
	enum {
		MTF_PrimaryColour = 1 << 0,
		MTF_SecondaryColour = 1 << 1
	};
	
	struct Material {
		std::vector<Texture> textures;
		glm::detail::tvec4<uint8_t> colour;
		
		uint8_t flags;
		
		float diffuseIntensity;
		float ambientIntensity;
	};
	
    struct SubGeometry {
		GLuint start = 0;
        size_t material;
		uint32_t* indices;
		size_t numIndices;
	};
	
	struct GeometryVertex {
		glm::vec3 position; /* 0 */
		glm::vec3 normal;   /* 24 */
		glm::vec2 texcoord; /* 48 */
		glm::vec4 colour;   /* 64 */
		
		static const AttributeList vertex_attributes() {
			return {
				{ATRS_Position, 3, sizeof(GeometryVertex),  0ul},
				{ATRS_Normal,   3, sizeof(GeometryVertex), sizeof(float)*3},
				{ATRS_TexCoord, 2, sizeof(GeometryVertex), sizeof(float)*6},
				{ATRS_Colour,   4, sizeof(GeometryVertex), sizeof(float)*8}
			};
		}
	};
	
	struct Geometry {
		DrawBuffer dbuff;
		GeometryBuffer gbuff;
		
		GLuint EBO;
		
		RW::BSGeometryBounds geometryBounds;
		
		uint32_t clumpNum;

        FaceType facetype;

		uint32_t flags;

		std::vector<Material> materials;
		std::vector<SubGeometry> subgeom;
		
		Geometry();
		~Geometry();
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