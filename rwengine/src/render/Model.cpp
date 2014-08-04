#include "render/Model.hpp"
#include <GL/glew.h>
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>


Model::Geometry::Geometry()
	: flags(0)
{
	
}

Model::Geometry::~Geometry()
{
	for(auto& sg : subgeom) {
		if( sg.indices ) {
			delete[] sg.indices;
		}
	}
}

ModelFrame::ModelFrame(ModelFrame* parent, glm::mat3 dR, glm::vec3 dT)
 : defaultRotation(dR), defaultTranslation(dT), parentFrame(parent)
{
	if(parent != nullptr) {
		parent->childs.push_back(this);
	}
	reset();
}

void ModelFrame::reset()
{
	matrix = glm::translate(glm::mat4(), defaultTranslation) * glm::mat4(defaultRotation);
}

void ModelFrame::addGeometry(size_t idx)
{
	geometries.push_back(idx);
}


Model::~Model()
{
	for(auto mf : frames) {
		delete mf;
	}
}
