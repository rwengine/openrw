#ifndef _VIEWCAMERA_HPP_
#define _VIEWCAMERA_HPP_
#include "ViewFrustum.hpp"
#include <glm/gtc/quaternion.hpp>

class ViewCamera
{
public:
	
	ViewFrustum frustum;
	
	glm::vec3 worldPos;

	ViewCamera()
		: frustum({0.1f, 5000.f, (-45.f / 180.f) * 3.1415f, 1.f})
	{
		
	}
};

#endif
