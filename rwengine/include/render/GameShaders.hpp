#pragma once
#ifndef _GAMESHADERS_HPP_
#define _GAMESHADERS_HPP_

namespace GameShaders {

struct WaterHQ {
	static const char* VertexShader;
	static const char* FragmentShader;
};
struct WaterLQ {
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

}

#endif
