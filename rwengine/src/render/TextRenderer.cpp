#include "render/TextRenderer.hpp"
#include <render/GameRenderer.hpp>
#include <engine/GameWorld.hpp>

const char* TextVertexShader = R"(
#version 130
#extension GL_ARB_explicit_attrib_location : enable
#extension GL_ARB_uniform_buffer_object : enable

layout(location = 0) in vec2 position;
layout(location = 3) in vec2 texcoord;
layout(location = 2) in vec3 colour;
out vec2 TexCoord;
out vec3 Colour;

uniform mat4 proj;

void main()
{
	gl_Position = proj * vec4(position, 0.0, 1.0);
	TexCoord = texcoord;
	Colour = colour;
})";

const char* TextFragmentShader = R"(
#version 130
in vec2 TexCoord;
in vec3 Colour;
uniform vec4 colour;
uniform sampler2D fontTexture;
out vec4 outColour;

void main()
{
	float a = texture(fontTexture, TexCoord).a;
	outColour = vec4(Colour, a);
})";

struct TextVertex
{
	glm::vec2 position;
	glm::vec2 texcoord;
	glm::vec3 colour;
	
	static const AttributeList vertex_attributes() {
		return {
			{ATRS_Position, 2, sizeof(TextVertex),  0ul},
			{ATRS_TexCoord, 2, sizeof(TextVertex),  0ul + sizeof(glm::vec2)},
			{ATRS_Colour, 3, sizeof(TextVertex),  0ul + sizeof(glm::vec2) * 2},
		};
	}
};


TextRenderer::TextInfo::TextInfo()
: font(0), size(1.f), baseColour({1.f, 1.f, 1.f}), align(Left)
{

}


TextRenderer::TextRenderer(GameWorld* engine, GameRenderer* renderer)
: fonts({}), engine(engine), renderer(renderer)
{
	textShader = renderer->getRenderer()->createShader(
		TextVertexShader, TextFragmentShader );
}

TextRenderer::~TextRenderer()
{

}

void TextRenderer::setFontTexture(int index, const std::string& texture)
{
	if( index < GAME_FONTS )
	{
		fonts[index] = texture;
	}
}

/// @todo This is very rough
int charToIndex(char g)
{
	if( g >= '0' && g <= '9' )
	{
		return 16 + (g - '0');
	}
	else if( g >= 'A' && g <= 'Z' )
	{
		return 33 + (g - 'A');
	}
	else if( g >= 'a' && g <= 'z' )
	{
		return 65 + (g - 'a');
	}
	switch(g)
	{
		default: return 0;
		case '!': return 1;
		case '"': return 2;
		case '#': return 3;
		case '$': return 4;
		case '%': return 5;
		case '&': return 6;
		case '\'': return 7;
		case '(': return 8;
		case ')': return 9;
		case '*': return 10;
		case '+': return 11;
		case ',': return 12;
		case '-': return 13;
		case '.': return 14;
		case '/': return 15;
	}
}

glm::vec4 indexToCoord(int font, int index)
{
	int x = index % 16;
	int y = index / 16;
	glm::vec2 gsize( 1.f / 16.f, 1.f / ((font == 0) ? 16.f : 13.f) );
	return glm::vec4( x, y, x + 1, y + 1 ) * 
	glm::vec4( gsize, gsize ); // + glm::vec4( 0.0001f, 0.0001f,-0.0001f,-0.0001f);
}

void TextRenderer::renderText(const TextRenderer::TextInfo& ti)
{
	renderer->getRenderer()->useProgram(textShader);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glActiveTexture(GL_TEXTURE0);
	
	renderer->getRenderer()->setUniform(textShader, "proj", renderer->getRenderer()->get2DProjection());
	renderer->getRenderer()->setUniformTexture(textShader, "fontTexture", 0);
	
	glm::vec2 coord  = ti.screenPosition;
	
	Renderer::DrawParameters dp;
	dp.start = 0;
	dp.count = gb.getCount();
	dp.texture = engine->gameData.textures[{fonts[ti.font], ""}].texName;
	glm::vec2 ss( ti.size );
	
	/// @todo smarter alignment
	if ( ti.align == TextInfo::Right )
	{
		coord.x -= ss.x * ti.text.length();
	}
	else if ( ti.align == TextInfo::Center )
	{
		coord.x -= glm::floor(ss.x * ti.text.length() * 0.5f);
	}
	
	glm::vec3 colour = ti.baseColour;
	
	/// @todo make this less wastefull
	for( const char& c : ti.text )
	{
		// Handle special chars.
		if( c == '\n' )
		{
			coord.x = ti.screenPosition.x;
			coord.y += ss.y;
			continue;
		}
		
		int glyph = charToIndex(c);
		auto tex = indexToCoord(ti.font, glyph);
		
		glm::vec2 p = coord;
		coord.x += ss.x;
		
		std::vector<TextVertex> geo = {
			{ { p.x,        p.y + ss.y }, {tex.x, tex.w}, colour },
			{ { p.x + ss.x, p.y + ss.y }, {tex.z, tex.w}, colour },
			{ { p.x,        p.y },        {tex.x, tex.y}, colour },
			{ { p.x + ss.x, p.y },        {tex.z, tex.y}, colour },
		};
		
		gb.uploadVertices(geo);
		db.addGeometry(&gb);
		db.setFaceType(GL_TRIANGLE_STRIP);
		
		renderer->getRenderer()->drawArrays(glm::mat4(), &db, dp);
	}
}
