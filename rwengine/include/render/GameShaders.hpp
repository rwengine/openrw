#pragma once
#ifndef _GAMESHADERS_HPP_
#define _GAMESHADERS_HPP_

/**
 * @brief collection of shaders to make managing them a little easier.
 */
namespace GameShaders {

struct WaterHQ {
	static const char* VertexShader;
	static const char* FragmentShader;
};

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

}

#endif
