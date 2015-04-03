#include "DrawUI.hpp"
#include <render/GameRenderer.hpp>
#include <ai/PlayerController.hpp>
#include <objects/CharacterObject.hpp>
#include <engine/GameState.hpp>
#include <glm/glm.hpp>

void drawMap(PlayerController* player, GameWorld* world, GameRenderer* render)
{
	MapRenderer::MapInfo map;
	map.scale = 0.4f;
	
	glm::quat plyRot;
	
	if( player )
	{
		plyRot = player->getCharacter()->getRotation();
	}
	
	map.rotation = glm::roll(plyRot);
	
	const glm::ivec2& vp = render->getRenderer()->getViewport();
	
	map.mapScreenTop = glm::vec2(260.f, vp.y - 10.f);
	map.mapScreenBottom = glm::vec2(10.f, vp.y - 210.f);
	
	if( player )
	{
		map.center = glm::vec2(player->getCharacter()->getPosition());
	}
	
	render->map.draw(map);
}

void drawHUD(PlayerController* player, GameWorld* world, GameRenderer* render)
{
	drawMap(player, world, render);
}

void drawOnScreenText(GameWorld* world)
{
	const glm::ivec2& vp = world->renderer.getRenderer()->getViewport();
	
	TextRenderer::TextInfo ti;
	ti.font = 2;
	ti.screenPosition = glm::vec2( 10.f, 10.f );
	ti.size = 20.f;
	
	for(OnscreenText& t : world->state.text)
	{
		glm::vec2 shadowOffset( 0, 0 );
		
		switch(t.osTextStyle)
		{
		default:
			ti.size = 15.f;
			ti.font = 0;
			ti.align = TextRenderer::TextInfo::Left;
			ti.baseColour = glm::vec3(1.f);
			ti.screenPosition = vp / 2;
			break;
		case OnscreenText::HighPriority:
			ti.size = 16.f;
			ti.font = 2;
			ti.baseColour = glm::vec3(1.f);
			ti.screenPosition = glm::vec2(vp.x * 0.5f, vp.y * 0.9f);
			ti.align = TextRenderer::TextInfo::Center;
			break;
		case OnscreenText::CenterBig:
			ti.size = 30.f;
			ti.font = 1;
			ti.baseColour = glm::vec3(82, 114, 128) / 255.f;
			ti.align = TextRenderer::TextInfo::Center;
			ti.screenPosition = vp / 2;
			ti.screenPosition += glm::vec2(0.f, ti.size / 2.f);
			shadowOffset = glm::vec2(2.f, 0.f);
			break;
		case OnscreenText::MissionName:
			ti.size = 30.f;
			ti.font = 1;
			ti.baseColour = glm::vec3(205, 162, 7)/255.f;
			ti.screenPosition = glm::vec2(vp.x - 10.f, vp.y * 0.79f);
			ti.align = TextRenderer::TextInfo::Right;
			shadowOffset = glm::vec2(2.f, 2.f);
			break;
		case OnscreenText::Help:
			ti.screenPosition = glm::vec2(20.f, 20.f);
			ti.font = 2;
			ti.size = 20.f;
			ti.baseColour = glm::vec3(1.f);
			ti.align = TextRenderer::TextInfo::Left;
			break;
		}

		ti.text = t.osTextString;

		if( t.osTextStyle == OnscreenText::Help )
		{
			// Insert line breaks into the message string.
			auto m = ti.text;
			const float boxWidth = 250.f;
			int lastSpace = 0;
			float lineLength = 0.f, wordLength = 0.f;
			for( int c = 0; c < m.length(); ++c )
			{
				if(m[c] == ' ')
				{
					lastSpace = c;
					lineLength += wordLength;
					wordLength = 0.f;
				}
				
				wordLength += ti.size;
				
				if( lineLength + wordLength > boxWidth )
				{
					m[lastSpace] = '\n';
					lineLength = 0.f;
				}
			}
			ti.text = m;
		}
		
		if( glm::length( shadowOffset  ) > 0 )
		{
			TextRenderer::TextInfo shadow( ti );
			shadow.baseColour = glm::vec3(0.f);
			shadow.screenPosition += shadowOffset;
			
			world->renderer.text.renderText(shadow);
		}
		
		world->renderer.text.renderText(ti);
	}

	for(auto& t : world->state.texts) {
		ti.font = 2;
		ti.screenPosition = t.position / glm::vec2(640, 480);
		ti.screenPosition *= vp;
		ti.baseColour = glm::vec3(t.colourFG);
		ti.size = 20.f;
		ti.text = t.text;
		
		world->renderer.text.renderText(ti);
	}
}
