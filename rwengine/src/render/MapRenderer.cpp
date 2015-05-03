#include <render/MapRenderer.hpp>
#include <render/GameShaders.hpp>
#include <engine/GameWorld.hpp>
#include <engine/GameData.hpp>
#include <engine/GameState.hpp>
#include <ai/PlayerController.hpp>
#include <objects/CharacterObject.hpp>

const char* MapVertexShader = R"(
#version 130
#extension GL_ARB_explicit_attrib_location : enable
#extension GL_ARB_uniform_buffer_object : enable

layout(location = 0) in vec2 position;
out vec2 TexCoord;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;

void main()
{
	gl_Position = proj * view * model * vec4(position, 0.0, 1.0);
	// UI space is top to bottom, so flip Y.
	TexCoord = position + vec2( 0.5 );
})";

const char* MapFragmentShader = R"(
#version 130
in vec2 TexCoord;
uniform vec4 colour;
uniform sampler2D spriteTexture;
out vec4 outColour;

void main()
{
	vec4 c = texture(spriteTexture, TexCoord);
	outColour = colour + c;
})";


MapRenderer::MapRenderer(Renderer* renderer, GameData* _data)
: data(_data), renderer(renderer)
{
	rectGeom.uploadVertices<VertexP2>({
		{-.5f,  .5f},
		{ .5f,  .5f},
		{-.5f, -.5f},
		{ .5f, -.5f}
	});
	rect.addGeometry(&rectGeom);
	rect.setFaceType(GL_TRIANGLE_STRIP);
	
	rectProg = renderer->createShader(
		MapVertexShader,
		MapFragmentShader
	);
	
	renderer->setUniform(rectProg, "colour", glm::vec4(1.f));
}

#define GAME_MAP_SIZE 4000

glm::vec2 MapRenderer::worldToMap(const glm::vec2& coord)
{
	return glm::vec2(coord.x, -coord.y);
}

glm::vec2 MapRenderer::mapToScreen(const glm::vec2& map, const MapInfo& mi)
{
	glm::vec2 screenSize = ((map-mi.center) * mi.scale);
	glm::vec2 screenCenter(500.f, 500.f);
	return screenSize + screenCenter;
}

void MapRenderer::draw(GameWorld* world, const MapInfo& mi)
{
	renderer->pushDebugGroup("Map");
	renderer->useProgram(rectProg);
	
	glm::vec2 bottom = glm::min(mi.mapScreenBottom, mi.mapScreenTop);
	glm::vec2 top = glm::max(mi.mapScreenBottom, mi.mapScreenTop);
	glm::vec2 screenPos = (bottom+top)/2.f;
	glm::vec2 scissorSize = top - bottom;
	const glm::ivec2& vp = renderer->getViewport();
	
	glEnable(GL_SCISSOR_TEST);
	glScissor(bottom.x, vp.y - bottom.y - scissorSize.y, scissorSize.x, scissorSize.y);
	
	auto proj = renderer->get2DProjection();
	glm::mat4 view, model;
	view = glm::translate(view, glm::vec3(screenPos, 0.f));
	view = glm::scale(view, glm::vec3(mi.scale));
	model = glm::rotate(model, mi.rotation, glm::vec3(0.f, 0.f, 1.f));
	model = glm::translate(model, glm::vec3(-worldToMap(mi.center), 0.f));
	renderer->setUniform(rectProg, "view", view);
	renderer->setUniform(rectProg, "proj", proj);
	
	glm::vec2 worldSize(GAME_MAP_SIZE);
	const int mapBlockLine = 8;
	glm::vec2 tileSize = worldSize / (float)mapBlockLine;
	
	glBindVertexArray( rect.getVAOName() );
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	renderer->setUniformTexture(rectProg, "spriteTexture", 0);

	glm::mat4 bg = glm::scale( model, glm::vec3( glm::vec2( worldSize ), 1.f ) );
	
	renderer->setUniform(rectProg, "model", bg);
	renderer->setUniform(rectProg, "colour", glm::vec4(0.0f, 0.0f, 0.0f, 1.f));

	glBindTexture(GL_TEXTURE_2D, 0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	
	// 2048, -2048
	
	renderer->setUniform(rectProg, "colour", glm::vec4(0.f));
	
	int initX = -(mapBlockLine/2);
	int initY = -(mapBlockLine/2);

	for( int m = 0; m < MAP_BLOCK_SIZE; ++m )
	{
		std::string num = (m < 10 ? "0" : "");
		std::string name = "radar" + num +  std::to_string(m);
		auto texture = world->data->textures[{name,""}];
		
		glBindTexture(GL_TEXTURE_2D, texture->getName());
		
		int mX = initX + (m % mapBlockLine);
		int mY = initY + (m / mapBlockLine);
		
		auto tc = glm::vec2(mX, mY) * tileSize + glm::vec2(tileSize/2.f);
		
		glm::mat4 tilemodel = model;
		tilemodel = glm::translate( tilemodel, glm::vec3( tc, 0.f ) );
		tilemodel = glm::scale( tilemodel, glm::vec3( tileSize, 1.f ) );
		
		renderer->setUniform(rectProg, "model", tilemodel);
		
		glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
	}
	
	glDisable(GL_SCISSOR_TEST);
	
	renderer->setUniform(rectProg, "view", view);
	
	for(auto& blip : world->state->radarBlips)
	{
		glm::vec2 blippos( blip.second.coord );
		if( blip.second.target > 0 )
		{
			auto object = world->findObject(blip.second.target);
			if( object )
			{
				blippos = glm::vec2( object->getPosition() );
			}
		}
		
		drawBlip(blippos, model, mi, "");
	}
	
	// Draw the player blip
	auto player = world->findObject(world->state->playerObject);
	if( player )
	{
		glm::vec2 plyblip(player->getPosition());
		float hdg = glm::roll(player->getRotation());
		drawBlip(plyblip, model, mi, "radar_centre", hdg);
	}
	
	glBindVertexArray( 0 );
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
	
	/// @TODO migrate to using the renderer
	renderer->invalidate();
	renderer->popDebugGroup();
}

void MapRenderer::drawBlip(const glm::vec2& coord, const glm::mat4& model, const MapInfo& mi, const std::string& texture, float heading)
{
	auto relPos = glm::vec2( model * glm::vec4( worldToMap(coord), 0.f, 1.f ) );
	
	// Now that relPos is relative to the rotation of the map, we can clip it.

	float invScale = 1.f/mi.scale;
	glm::vec2 mapCenter( (mi.mapScreenTop + mi.mapScreenBottom) / 2.f );
	glm::vec2 mapMin( (mi.mapScreenBottom - mapCenter) );
	glm::vec2 mapMax( (mi.mapScreenTop - mapCenter) );
	
	relPos = glm::max( mapMin * invScale, glm::min( mapMax * invScale, relPos ) );
	glm::vec2 map = glm::vec2( glm::inverse(model) * glm::vec4( relPos, 0.f, 1.f ) );

	glm::mat4 m = model;
	m = glm::translate(m, glm::vec3(map, 0.f));
	m = glm::scale(m, glm::vec3(16.f * 1.f/mi.scale));
	m = glm::rotate(m, heading, glm::vec3(0.f, 0.f, -1.f));

	renderer->setUniform(rectProg, "model", m);
	
	GLuint tex = 0;
	if ( !texture.empty() )
	{
		auto sprite= data->findTexture(texture);
		tex = sprite->getName();
		renderer->setUniform(rectProg, "colour", glm::vec4(0.f));
	}
	else
	{
		renderer->setUniform(rectProg, "colour", glm::vec4(1.0f, 1.0f, 1.0f, 1.f));
	}
	
	glBindTexture(GL_TEXTURE_2D, tex);

	glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
}
