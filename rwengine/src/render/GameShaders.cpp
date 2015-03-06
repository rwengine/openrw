#include <render/GameShaders.hpp>

namespace GameShaders {

const char* WaterHQ::VertexShader = R"(
#version 130
#extension GL_ARB_explicit_attrib_location : enable
#extension GL_ARB_uniform_buffer_object : enable

layout(location = 0) in vec2 position;
out vec2 TexCoords;

layout(std140) uniform SceneData {
	mat4 projection;
	mat4 view;
	vec4 ambient;
	vec4 dynamic;
	vec4 fogColor;
	vec4 campos;
	float fogStart;
	float fogEnd;
};

layout(std140) uniform ObjectData {
	mat4 model;
	vec4 colour;
	float diffusefac;
	float ambientfac;
	float visibility;
};

uniform float size;

uniform float time;
uniform vec2 waveParams;

void main()
{
	mat4 MVP = projection * view;
	vec4 vp = model * vec4(position * size, 0.0, 1.0);
	vp.z = (1.0+sin(time + (vp.x + vp.y) * waveParams.x)) * waveParams.y;
	TexCoords = position * 2.0;
	gl_Position = MVP * vp;
})";

const char* WaterHQ::FragmentShader = R"(
#version 130
in vec3 Normal;
in vec2 TexCoords;
uniform sampler2D texture;
out vec4 outColour;
void main() {
	vec4 c = texture2D(texture, TexCoords);
	outColour = c;
})";

const char* Sky::VertexShader = R"(
#version 130
#extension GL_ARB_explicit_attrib_location : enable
#extension GL_ARB_uniform_buffer_object : enable

layout(std140) uniform SceneData {
	mat4 projection;
	mat4 view;
	vec4 ambient;
	vec4 dynamic;
	vec4 fogColor;
	vec4 campos;
	float fogStart;
	float fogEnd;
};

layout(location = 0) in vec3 position;
out vec3 Position;

void main() {
	Position = position;
	vec4 viewsp = projection * mat4(mat3(view)) * vec4(position, 1.0);
	viewsp.z = viewsp.w - 0.000001;
	gl_Position = viewsp;
})";

const char* Sky::FragmentShader = R"(
#version 130
in vec3 Position;
uniform vec4 TopColor;
uniform vec4 BottomColor;
out vec4 outColour;
void main() {
	outColour = mix(BottomColor, TopColor, clamp(Position.z, 0, 1));
})";

/*
 * GLSL 130 is required until the game can run in core context,
 * SFML needs to up it's game.
 */

const char* WorldObject::VertexShader = R"(
#version 130
#extension GL_ARB_explicit_attrib_location : enable
#extension GL_ARB_uniform_buffer_object : enable
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 _colour;
layout(location = 3) in vec2 texCoords;
out vec3 Normal;
out vec2 TexCoords;
out vec4 Colour;
out vec3 WorldSpace;

layout(std140) uniform SceneData {
	mat4 projection;
	mat4 view;
	vec4 ambient;
	vec4 dynamic;
	vec4 fogColor;
	vec4 campos;
	float fogStart;
	float fogEnd;
};

layout(std140) uniform ObjectData {
	mat4 model;
	vec4 colour;
	float diffusefac;
	float ambientfac;
	float visibility;
};

void main()
{
	Normal = normal;
	TexCoords = texCoords;
	Colour = _colour;
	vec4 worldspace = model * vec4(position, 1.0);
	vec4 viewspace = view * worldspace;
	gl_Position = projection * viewspace;

	WorldSpace = worldspace.xyz;
})";

const char* WorldObject::FragmentShader = R"(
#version 130
#extension GL_ARB_uniform_buffer_object : enable
in vec3 Normal;
in vec2 TexCoords;
in vec4 Colour;
in vec3 WorldSpace;
uniform sampler2D texture;
out vec4 fragOut;

layout(std140) uniform SceneData {
	mat4 projection;
	mat4 view;
	vec4 ambient;
	vec4 dynamic;
	vec4 fogColor;
	vec4 campos;
	float fogStart;
	float fogEnd;
};

layout(std140) uniform ObjectData {
	mat4 model;
	vec4 colour;
	float diffusefac;
	float ambientfac;
	float visibility;
};

#define ALPHA_DISCARD_THRESHOLD 0.01

vec4 crunch(vec4 c)
{
	return vec4(0.5) + c * 0.5;
}

void main()
{
	vec4 c = texture2D(texture, TexCoords);
	if(c.a <= ALPHA_DISCARD_THRESHOLD) discard;
	float fogZ = length(campos.xyz - WorldSpace.xyz);
	float fogfac = 1.0 - clamp( (fogEnd-fogZ)/(fogEnd-fogStart), 0.0, 1.0 );
	vec3 tmp = (ambient.rgb + c.rgb) * crunch(Colour).rgb;
	vec4 diffuseC = vec4(colour.rgb * tmp, c.a * colour.a);
	vec2 cscale = vec2(1.0, visibility);
	fragOut = cscale.xxxy *	(fogfac * fogColor + (1.0-fogfac) * diffuseC);
})";

const char* Particle::FragmentShader = R"(
#version 130
#extension GL_ARB_uniform_buffer_object : enable
in vec3 Normal;
in vec2 TexCoords;
in vec4 Colour;
uniform sampler2D texture;
out vec4 outColour;

layout(std140) uniform SceneData {
	mat4 projection;
	mat4 view;
	vec4 ambient;
	vec4 dynamic;
	vec4 fogColor;
	vec4 campos;
	float fogStart;
	float fogEnd;
};

layout(std140) uniform ObjectData {
	mat4 model;
	vec4 colour;
	float diffusefac;
	float ambientfac;
	float visibility;
};

#define ALPHA_DISCARD_THRESHOLD 0.01

void main()
{
	vec4 c = texture2D(texture, TexCoords);
	c.a = clamp(0, length(c.rgb/length(vec3(1,1,1))), 1);
	if(c.a <= ALPHA_DISCARD_THRESHOLD) discard;
	float fogZ = (gl_FragCoord.z / gl_FragCoord.w);
	float fogfac = clamp( (fogStart-fogZ)/(fogEnd-fogStart), 0.0, 1.0 );
	vec4 tint = vec4(colour.rgb, visibility);
	outColour = c * tint;
})";


const char* ScreenSpaceRect::VertexShader = R"(
#version 130
#extension GL_ARB_explicit_attrib_location : enable
#extension GL_ARB_uniform_buffer_object : enable

layout(location = 0) in vec2 position;
out vec2 TexCoords;
out vec4 Colour;
uniform vec2 size;
uniform vec2 offset;

void main()
{
	TexCoords = position * vec2(0.5,-0.5) + vec2(0.5);
	gl_Position = vec4(offset + position * size, 0.0, 1.0);
})";

const char* ScreenSpaceRect::FragmentShader = R"(
#version 130
in vec2 TexCoords;
in vec4 Colour;
uniform vec4 colour;
uniform sampler2D texture;
out vec4 outColour;

void main()
{
	vec4 c = texture2D(texture, TexCoords);
	// Set colour to 0, 0, 0, 1 for textured mode.
	outColour = vec4(colour.rgb + c.rgb, colour.a);
})";
}
