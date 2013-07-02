#ifndef _VIEWCAMERA_HPP_
#define _VIEWCAMERA_HPP_
#include "ViewFrustum.hpp"
#include <glm/gtc/quaternion.hpp>

class ViewCamera
{
public:
	
	ViewFrustum frustum;
	
	glm::mat4 view;
	
	ViewCamera()
	: frustum({0.1f, 5000.f, 80.f, 1.f})
	{
		
	}
};

#endif