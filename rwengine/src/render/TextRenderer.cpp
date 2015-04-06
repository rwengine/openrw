#include "render/TextRenderer.hpp"
#include <render/GameRenderer.hpp>
#include <engine/GameWorld.hpp>
#include <boost/concept_check.hpp>

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
uniform vec2 alignment;

void main()
{
	gl_Position = proj * vec4(alignment + position, 0.0, 1.0);
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
	
	for( int g = 0; g < GAME_GLYPHS; g++ )
	{
		glyphData[g] = { 1.f };
	}
	
	glyphData[charToIndex(' ')].widthFrac = 0.4f;
	glyphData[charToIndex('\'')].widthFrac = 0.5f;
	// Assumes contigious a-z character encoding
	for(char g = 0; g <= ('z'-'a'); g++)
	{
		switch( ('a' + g) )
		{
		case 'i':
			glyphData[charToIndex('a' + g)].widthFrac = 0.4f;
			glyphData[charToIndex('A' + g)].widthFrac = 0.4f;
			break;
		case 'l':
			glyphData[charToIndex('a' + g)].widthFrac = 0.5f;
			glyphData[charToIndex('A' + g)].widthFrac = 0.5f;
			break;
		case 'm':
			glyphData[charToIndex('a' + g)].widthFrac = 1.0f;
			glyphData[charToIndex('A' + g)].widthFrac = 1.0f;
			break;
		case 'w':
			glyphData[charToIndex('a' + g)].widthFrac = 1.0f;
			glyphData[charToIndex('A' + g)].widthFrac = 1.0f;
			break;
		default:
			glyphData[charToIndex('a' + g)].widthFrac = 0.7f;
			glyphData[charToIndex('A' + g)].widthFrac = 0.7f;
			break;
		}
	}
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

void TextRenderer::renderText(const TextRenderer::TextInfo& ti)
{
	renderer->getRenderer()->pushDebugGroup("Text");
	renderer->getRenderer()->useProgram(textShader);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glActiveTexture(GL_TEXTURE0);
	
	glm::vec2 coord( 0.f, 0.f );
	glm::vec2 alignment = ti.screenPosition;
	
	glm::vec2 ss( ti.size );

	glm::vec3 colour = ti.baseColour;
	std::vector<TextVertex> geo;
	
	float maxWidth = 0.f;
	
	for( const char& c : ti.text )
	{
		int glyph = charToIndex(c);
		if( glyph >= GAME_GLYPHS )
		{
			continue;
		}
		
		auto& data = glyphData[glyph];
		auto tex = indexToCoord(ti.font, glyph);
		
		ss.x = ti.size * data.widthFrac;
		tex.z = tex.x + (tex.z - tex.x) * data.widthFrac;
		
		// Handle special chars.
		if( c == '\n' )
		{
			coord.x = ti.screenPosition.x;
			coord.y += ss.y;
			continue;
		}
		
		glm::vec2 p = coord;
		coord.x += ss.x;
		maxWidth = std::max(coord.x, maxWidth);
		
		geo.push_back({ { p.x,        p.y + ss.y }, {tex.x, tex.w}, colour });
		geo.push_back({ { p.x + ss.x, p.y + ss.y }, {tex.z, tex.w}, colour });
		geo.push_back({ { p.x,        p.y },        {tex.x, tex.y}, colour });
		
		
		geo.push_back({ { p.x + ss.x, p.y },        {tex.z, tex.y}, colour });
		geo.push_back({ { p.x,        p.y },        {tex.x, tex.y}, colour });
		geo.push_back({ { p.x + ss.x, p.y + ss.y }, {tex.z, tex.w}, colour });
	}
	
	if ( ti.align == TextInfo::Right )
	{
		alignment.x -= maxWidth;
	}
	else if ( ti.align == TextInfo::Center )
	{
		alignment.x -= (maxWidth / 2.f);
	}
	
	renderer->getRenderer()->setUniform(textShader, "proj", renderer->getRenderer()->get2DProjection());
	renderer->getRenderer()->setUniformTexture(textShader, "fontTexture", 0);
	renderer->getRenderer()->setUniform(textShader, "alignment", alignment);
	
	gb.uploadVertices(geo);
	db.addGeometry(&gb);
	db.setFaceType(GL_TRIANGLES);
	
	Renderer::DrawParameters dp;
	dp.start = 0;
	dp.count = gb.getCount();
	auto ftexture = engine->gameData.findTexture(fonts[ti.font]);
	dp.texture = ftexture->getName();
	
	renderer->getRenderer()->drawArrays(glm::mat4(), &db, dp);

	renderer->getRenderer()->popDebugGroup();
}
