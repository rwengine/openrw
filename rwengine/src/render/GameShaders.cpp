#include <render/GameShaders.hpp>

namespace GameShaders {

const char* WaterHQ::VertexShader = R"(
#version 130
#extension GL_ARB_explicit_attrib_location : enable
layout(location = 0) in vec2 position;
out vec2 TexCoords;
uniform float height;
uniform float size;
uniform mat4 MVP;
uniform float time;
uniform vec2 worldP;
uniform vec2 waveParams;
void main()
{
	vec2 p = worldP + position * size;
	float waveHeight = (1.0+sin(time + (p.x + p.y) * waveParams.x)) * waveParams.y;
	TexCoords = position * 2.0;
	gl_Position = MVP * vec4(p, height + waveHeight, 1.0);
})";

const char* WaterHQ::FragmentShader = R"(
#version 130
in vec3 Normal;
in vec2 TexCoords;
uniform sampler2D texture;
void main() {
	vec4 c = texture2D(texture, TexCoords);
	gl_FragColor = c;
})";

const char* Sky::VertexShader = R"(
#version 130
in vec3 position;
uniform mat4 view;
uniform mat4 proj;
out vec3 Position;
uniform float Far;
void main() {
	Position = position;
	vec4 viewsp = proj * mat4(mat3(view)) * vec4(position, 1.0);
	viewsp.z = viewsp.w - 0.000001;
	gl_Position = viewsp;
})";

const char* Sky::FragmentShader = R"(
#version 130
in vec3 Position;
uniform vec4 TopColor;
uniform vec4 BottomColor;
void main() {
	gl_FragColor = mix(BottomColor, TopColor, clamp(Position.z, 0, 1));
})";

const char* WorldObject::VertexShader = R"(
#version 130
#extension GL_ARB_explicit_attrib_location : enable
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 colour;
layout(location = 3) in vec2 texCoords;
out vec3 Normal;
out vec2 TexCoords;
out vec4 Colour;
out vec4 EyeSpace;
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
void main()
{
	Normal = normal;
	TexCoords = texCoords;
	Colour = colour;
	vec4 eyeSpace = view * model * vec4(position, 1.0);
	EyeSpace = proj * eyeSpace;
	gl_Position = proj * eyeSpace;
})";

const char* WorldObject::FragmentShader = R"(
#version 130
in vec3 Normal;
in vec2 TexCoords;
in vec4 Colour;
in vec4 EyeSpace;
uniform sampler2D texture;
uniform vec4 BaseColour;
uniform vec4 AmbientColour;
uniform vec4 DynamicColour;
uniform vec3 SunDirection;
uniform float FogStart;
uniform float FogEnd;
uniform float MaterialDiffuse;
uniform float MaterialAmbient;
void main()
{
	vec4 c = texture2D(texture, TexCoords);
	if(c.a < 0.1) discard;
	float fogZ = (gl_FragCoord.z / gl_FragCoord.w);
	float fogfac = clamp( (FogEnd-fogZ)/(FogEnd-FogStart), 0.0, 1.0 );
	//	gl_FragColor = mix(AmbientColour, c, fogfac);
	gl_FragColor = mix(AmbientColour, BaseColour * (vec4(0.5) + Colour * 0.5) * (vec4(0.5) + DynamicColour * 0.5) * c, fogfac);
})";

}
