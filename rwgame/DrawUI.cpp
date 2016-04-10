#include "DrawUI.hpp"
#include <render/GameRenderer.hpp>
#include <ai/PlayerController.hpp>
#include <objects/CharacterObject.hpp>
#include <engine/GameState.hpp>
#include <items/WeaponItem.hpp>

#include <iomanip>

constexpr size_t ui_textSize = 25;
constexpr size_t ui_textHeight = 22;
constexpr size_t ui_outerMargin = 20;
constexpr size_t ui_infoMargin = 10;
constexpr size_t ui_weaponSize = 64;
constexpr size_t ui_ammoSize = 14;
constexpr size_t ui_ammoHeight = 16;
constexpr size_t ui_armourOffset = ui_textSize * 3;
constexpr size_t ui_maxWantedLevel = 6;
#define RGB_COLOR(r,g,b) r/255.f, g/255.f, b/255.f
const glm::vec3 ui_timeColour(RGB_COLOR(196, 165, 119));
const glm::vec3 ui_moneyColour(RGB_COLOR(89, 113, 147));
const glm::vec3 ui_healthColour(RGB_COLOR(187, 102, 47));
const glm::vec3 ui_armourColour(RGB_COLOR(123, 136, 93));

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
	
	render->map.draw(world, map);
}

void drawPlayerInfo(PlayerController* player, GameWorld* world, GameRenderer* render)
{
	float infoTextX = render->getRenderer()->getViewport().x -
			(ui_outerMargin + ui_weaponSize + ui_infoMargin);
	float infoTextY = 0.f + ui_outerMargin;
	float iconX = render->getRenderer()->getViewport().x -
			(ui_outerMargin + ui_weaponSize);
	float iconY = ui_outerMargin;
	float wantedX = render->getRenderer()->getViewport().x -
			(ui_outerMargin);
	float wantedY = ui_outerMargin + ui_weaponSize + 3.f;

	TextRenderer::TextInfo ti;
	ti.font = 1;
	ti.size = ui_textSize;
	ti.align = TextRenderer::TextInfo::Right;

	{
		std::stringstream ss;
		ss << std::setw(2) << std::setfill('0') << world->getHour()
		   << std::setw(0) << ":"
		   << std::setw(2) << world->getMinute();

		ti.text = ss.str();
	}
	ti.baseColour = glm::vec3(0.f, 0.f, 0.f);
	ti.screenPosition = glm::vec2(infoTextX + 1.f, infoTextY+1.f);
	render->text.renderText(ti);

	ti.baseColour = ui_timeColour;
	ti.screenPosition = glm::vec2(infoTextX, infoTextY);
	render->text.renderText(ti);

	infoTextY += ui_textHeight;

	ti.text = "$" + std::to_string(world->state->playerInfo.money);
	ti.baseColour = glm::vec3(0.f, 0.f, 0.f);
	ti.screenPosition = glm::vec2(infoTextX + 1.f, infoTextY+1.f);
	render->text.renderText(ti);

	ti.baseColour = ui_moneyColour;
	ti.screenPosition = glm::vec2(infoTextX, infoTextY);
	render->text.renderText(ti);

	infoTextY += ui_textHeight;

	{
		std::stringstream ss;
		ss << "@" << std::setw(3) << std::setfill('0')
		   << player->getCharacter()->getCurrentState().health;
		ti.text = ss.str();
	}
	ti.baseColour = glm::vec3(0.f, 0.f, 0.f);
	ti.screenPosition = glm::vec2(infoTextX + 1.f, infoTextY+1.f);
	render->text.renderText(ti);

	ti.baseColour = ui_healthColour;
	ti.screenPosition = glm::vec2(infoTextX, infoTextY);
	render->text.renderText(ti);

	if (player->getCharacter()->getCurrentState().armour > 0)
	{
		std::stringstream ss;
		ss << "[" << std::setw(3) << std::setfill('0')
		   << player->getCharacter()->getCurrentState().armour;
		ti.text = ss.str();
		ti.baseColour = glm::vec3(0.f, 0.f, 0.f);
		ti.screenPosition = glm::vec2(infoTextX + 1.f - ui_armourOffset, infoTextY+1.f);
		render->text.renderText(ti);

		ti.baseColour = ui_armourColour;
		ti.screenPosition = glm::vec2(infoTextX - ui_armourOffset, infoTextY);
		render->text.renderText(ti);
	}

	std::string s;
	for (int i = 0; i < ui_maxWantedLevel; ++i) {
		s += "]";
	}
	ti.text = s;
	ti.baseColour = glm::vec3(0.f, 0.f, 0.f);
	ti.screenPosition = glm::vec2(wantedX + 1.f, wantedY + 1.f);
	render->text.renderText(ti);

#if 0 // Useful for debugging
	ti.text = "ABCDEFGHIJKLMANOQRTSWXYZ\nM0123456789";
	ti.size = 30;
	ti.align = TextRenderer::TextInfo::Left;
	ti.baseColour = glm::vec3(0.f, 0.f, 0.f);
	ti.screenPosition = glm::vec2(101.f, 202.f);
	render->text.renderText(ti);
	ti.baseColour = glm::vec3(1.f, 1.f, 1.f);
	ti.screenPosition = glm::vec2(100.f, 200.f);
	render->text.renderText(ti);
#endif

	InventoryItem *current = player->getCharacter()->getActiveItem();
	std::string itemTextureName = "fist";
	if (current) {
		uint16_t model = current->getModelID();
		if (model > 0) {
			ObjectDataPtr weaponData = world
					->data
					->findObjectType<ObjectData>(model);
			if (weaponData != nullptr) {
				itemTextureName = weaponData->modelName;
			}
		}
	}
	// Urgh
	if (itemTextureName == "colt45") {
		itemTextureName = "pistol";
	}

	TextureData::Handle itemTexture = render->getData()->findTexture(itemTextureName);
	RW_CHECK(itemTexture != nullptr, "Item has 0 texture");
	if (itemTexture != nullptr) {
		RW_CHECK(itemTexture->getName() != 0, "Item has 0 texture");
		render->drawTexture(itemTexture.get(),
							glm::vec4(iconX,
									  iconY,
									  ui_weaponSize,
									  ui_weaponSize));
	}

	if (current) {
		WeaponItem* wep = static_cast<WeaponItem*>(current);
		if (wep->getWeaponData()->fireType != WeaponData::MELEE) {
			const CharacterState& cs = player->getCharacter()->getCurrentState();
			const CharacterWeaponSlot& slotInfo = cs.weapons[cs.currentWeapon];
			ti.text = std::to_string(slotInfo.bulletsClip) + "-"
					+ std::to_string(slotInfo.bulletsTotal);

			ti.baseColour = glm::vec3(0.f, 0.f, 0.f);
			ti.font = 2;
			ti.size = ui_ammoSize;
			ti.align = TextRenderer::TextInfo::Center;
			ti.screenPosition = glm::vec2(iconX + ui_weaponSize / 2.f,
										  iconY + ui_weaponSize - ui_ammoHeight);
			render->text.renderText(ti);
		}
	}
}

void drawHUD(PlayerController* player, GameWorld* world, GameRenderer* render)
{
	drawMap(player, world, render);
	drawPlayerInfo(player, world, render);
}

void drawOnScreenText(GameWorld* world, GameRenderer* renderer)
{
	const glm::ivec2& vp = renderer->getRenderer()->getViewport();
	
	TextRenderer::TextInfo ti;
	ti.font = 2;
	ti.screenPosition = glm::vec2( 10.f, 10.f );
	ti.size = 20.f;
	
	for(OnscreenText& t : world->state->text)
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
			ti.baseColour = glm::vec3(0.9f);
			ti.align = TextRenderer::TextInfo::Left;
			break;
		}

		ti.text = t.osTextString;
		ti.varText = t.osTextVar;
		
		if( glm::length( shadowOffset  ) > 0 )
		{
			TextRenderer::TextInfo shadow( ti );
			shadow.baseColour = glm::vec3(0.f);
			shadow.screenPosition += shadowOffset;
			
			renderer->text.renderText(shadow);
		}
		
		renderer->text.renderText(ti);
	}

	for(auto& t : world->state->texts) {
		ti.font = 2;
		ti.screenPosition = t.position / glm::vec2(640, 480);
		ti.screenPosition *= vp;
		ti.baseColour = glm::vec3(t.colourFG);
		ti.size = 20.f;
		ti.text = t.text;
		
		renderer->text.renderText(ti);
	}
}
