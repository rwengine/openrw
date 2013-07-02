#ifndef _GTARENDERER_HPP_
#define _GTARENDERER_HPP_
#include "ViewCamera.hpp"

class GTAEngine;
class GTARenderer
{
public:
	
	GTARenderer();
	
	ViewCamera camera;
	
	void renderWorld(GTAEngine* engine);
};

#endif