#pragma once
#ifndef _GAMESHADERS_HPP_
#define _GAMESHADERS_HPP_

#define SHADER_VF(Name) \
struct Name {\
	static const char* VertexShader;\
	static const char* FragmentShader;\
}

/**
 * @brief collection of shaders to make managing them a little easier.
 */
namespace GameShaders {

/**
 * High Quality Projected-Grid water shader
 */
SHADER_VF(WaterHQ);

/**
 * Simple 3D masking shader
 */
SHADER_VF(Mask3D);

struct Sky {
	static const char* VertexShader;
	static const char* FragmentShader;
};

struct WorldObject {
	static const char* VertexShader;
	static const char* FragmentShader;
};

/** @brief Particle effect shaders, uses WorldObject::VertexShader */
struct Particle {
	static const char* FragmentShader;
};

/**
 * @brief The ScreenSpaceRect shader
 *
 * Used to draw black bars, splash screens, fading etc.
 */
struct ScreenSpaceRect {
	static const char* VertexShader;
	static const char* FragmentShader;
};

SHADER_VF(DefaultPostProcess);

}

#endif
