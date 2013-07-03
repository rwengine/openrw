#ifndef _GTARENDERER_HPP_
#define _GTARENDERER_HPP_

#define GLEW_STATIC
#include <GL/glew.h>

#include <renderwure/render/ViewCamera.hpp>

class GTAEngine;
class GTARenderer
{
public:
	
	GTARenderer();
	
	ViewCamera camera;
	
	/// The numer of things rendered by the last renderWorld
	size_t rendered;
	size_t culled;

	GLuint uniModel, uniProj, uniView;
	GLuint posAttrib, texAttrib;
	GLuint worldProgram;
	
	void renderWorld(GTAEngine* engine);
};

#endif