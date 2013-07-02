#ifndef _GTARENDERER_HPP_
#define _GTARENDERER_HPP_
#include "ViewCamera.hpp"

class GTAEngine;
class GTARenderer
{
public:
	
	GTARenderer();
	
	ViewCamera camera;
	
	/// The numer of things rendered by the last renderWorld
	size_t rendered;
	size_t culled;
	
	void renderWorld(GTAEngine* engine);
};

#endif