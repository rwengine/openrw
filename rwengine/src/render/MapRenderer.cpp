#include <render/MapRenderer.hpp>
#include <render/GameShaders.hpp>
#include <engine/GameWorld.hpp>
#include <ai/PlayerController.hpp>
#include <objects/CharacterObject.hpp>

MapRenderer::MapRenderer(GameWorld* world, Renderer* renderer)
: world( world ), renderer(renderer)
{
	rectGeom.uploadVertices<VertexP2>({
		{-1.f, -1.f},
		{ 1.f, -1.f},
		{-1.f,  1.f},
		{ 1.f,  1.f}
	});
	rect.addGeometry(&rectGeom);
	rect.setFaceType(GL_TRIANGLE_STRIP);
	
	rectProg = renderer->createShader(
		GameShaders::ScreenSpaceRect::VertexShader,
		GameShaders::ScreenSpaceRect::FragmentShader
	);
	
	renderer->setUniform(rectProg, "colour", glm::vec4(1.f));
	renderer->setUniform(rectProg, "size", glm::vec2(1.f, 1.f));
	renderer->setUniform(rectProg, "offset", glm::vec2(1.f, 1.f));
}

#define GAME_MAP_SIZE 4048

glm::vec2 MapRenderer::mapToScreen(const glm::vec2& map, const MapInfo& mi)
{
	glm::vec2 screenSize = ((map-mi.center) * mi.scale) / mi.viewport;
	glm::vec2 screenCenter = mi.mapPos / (mi.viewport/2.f) - 1.f;
	return screenSize + screenCenter;
}

glm::vec2 MapRenderer::sizeOnScreen(const glm::vec2& map, const MapRenderer::MapInfo& mi)
{
	glm::vec2 screenSize = ((map) * mi.scale) / mi.viewport;	
	return screenSize;
}

void MapRenderer::draw(const MapInfo& mi)
{
	const glm::vec2& viewport = mi.viewport;
	const glm::vec2& offset = -mi.center;
	
	renderer->useProgram(rectProg);
	
	glm::vec2 worldSize(GAME_MAP_SIZE);
	const int mapBlockLine = 8;
	glm::vec2 tileSize = worldSize / (float)mapBlockLine;
	glm::vec2 tileScreenSize = sizeOnScreen(tileSize, mi);
	glm::vec2 basePos = mi.mapPos - (mi.mapSize/2.f);
	
	glEnable(GL_SCISSOR_TEST);
	glScissor(basePos.x, basePos.y, mi.mapSize.x, mi.mapSize.y);
	
	glBindVertexArray( rect.getVAOName() );
	glActiveTexture(GL_TEXTURE0);
	
	// Draw backdrop
	renderer->setUniform(rectProg, "size", sizeOnScreen(worldSize/2.f, mi));
	renderer->setUniform(rectProg, "colour", glm::vec4(0.0f, 0.0f, 0.0f, 1.f));
	renderer->setUniform(rectProg, "offset", mapToScreen(glm::vec2(0.f, 0.f), mi));
	glBindTexture(GL_TEXTURE_2D, 0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	
	// 2048, -2048
	
	renderer->setUniform(rectProg, "size", tileScreenSize/2.f);
	renderer->setUniform(rectProg, "colour", glm::vec4(0.0f, 0.0f, 0.0f, 0.f));
	int initX = -(mapBlockLine/2);
	int initY = (mapBlockLine/2);
	for( int m = 0; m < MAP_BLOCK_SIZE; ++m )
	{
		std::string num = (m < 10 ? "0" : "");
		std::string name = "radar" + num +  std::to_string(m);
		auto texture = world->gameData.textures[{name,""}];
		
		glBindTexture(GL_TEXTURE_2D, texture.texName);
		renderer->setUniformTexture(rectProg, "texture", 0);
		
		int mX = initX + (m % mapBlockLine);
		int mY = initY - (m / mapBlockLine);
		auto tc = glm::vec2(mX, mY) * tileSize + (glm::vec2(tileSize.x,-tileSize.y)/2.f);
		
		auto screen = mapToScreen(tc, mi);
		
		renderer->setUniform(rectProg, "offset", screen);
		
		glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
	}
	
	// Draw the player blip
	auto player = world->state.player;
	if( player )
	{
		glm::vec2 plyblip(player->getCharacter()->getPosition());
		drawBlip(plyblip, mi, "radar_centre");
	}
	
	glDisable(GL_SCISSOR_TEST);
	
	glBindVertexArray( 0 );
	glBindTexture(GL_TEXTURE_2D, 0);
	glUseProgram(0);
}

void MapRenderer::drawBlip(const glm::vec2& map, const MapInfo& mi, const std::string& texture)
{
	glm::vec2 screen = mapToScreen(map, mi);
	
	auto sprite = world->gameData.textures[{texture,""}];
	glBindTexture(GL_TEXTURE_2D, sprite.texName);

	renderer->setUniform(rectProg, "size", 16.f/mi.viewport);
	
	renderer->setUniform(rectProg, "offset", screen);
	glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
}
