#include "DrawUI.hpp"
#include <ai/PlayerController.hpp>
#include <engine/GameState.hpp>
#include <objects/CharacterObject.hpp>
#include <data/WeaponData.hpp>
#include <render/GameRenderer.hpp>

#include <glm/gtc/constants.hpp>
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
const glm::u8vec3 ui_timeColour(196, 165, 119);
const glm::u8vec3 ui_moneyColour(89, 113, 147);
const glm::u8vec3 ui_healthColour(187, 102, 47);
const glm::u8vec3 ui_armourColour(123, 136, 93);
const glm::u8vec3 ui_shadowColour(0, 0, 0);
const float ui_mapSize = 150.f;
const float ui_worldSizeMin = 200.f;
const float ui_worldSizeMax = 300.f;

void drawMap(ViewCamera& currentView, PlayerController* player,
             GameWorld* world, GameRenderer* render) {
    MapRenderer::MapInfo map;

    glm::quat camRot = currentView.rotation;

    map.rotation = glm::roll(camRot) - glm::half_pi<float>();
    map.worldSize = ui_worldSizeMin;
    map.worldSize = ui_worldSizeMax;
    if (player) {
        map.worldCenter = glm::vec2(player->getCharacter()->getPosition());
    }

    const glm::ivec2& vp = render->getRenderer()->getViewport();

    glm::vec2 mapTop =
        glm::vec2(ui_outerMargin, vp.y - (ui_outerMargin + ui_mapSize));
    glm::vec2 mapBottom =
        glm::vec2(ui_outerMargin + ui_mapSize, vp.y - ui_outerMargin);

    map.screenPosition = (mapTop + mapBottom) / 2.f;
    map.screenSize = ui_mapSize * 0.95;

    render->map.draw(world, map);
}

void drawPlayerInfo(PlayerController* player, GameWorld* world,
                    GameRenderer* render) {
    float infoTextX = render->getRenderer()->getViewport().x -
                      (ui_outerMargin + ui_weaponSize + ui_infoMargin);
    float infoTextY = 0.f + ui_outerMargin;
    float iconX = render->getRenderer()->getViewport().x -
                  (ui_outerMargin + ui_weaponSize);
    float iconY = ui_outerMargin;
    float wantedX = render->getRenderer()->getViewport().x - (ui_outerMargin);
    float wantedY = ui_outerMargin + ui_weaponSize + 3.f;

    TextRenderer::TextInfo ti;
    ti.font = 1;
    ti.size = ui_textSize;
    ti.align = TextRenderer::TextInfo::Right;

    {
        std::stringstream ss;
        ss << std::setw(2) << std::setfill('0') << world->getHour()
           << std::setw(0) << ":" << std::setw(2) << world->getMinute();

        ti.text = GameStringUtil::fromString(ss.str());
    }
    ti.baseColour = ui_shadowColour;
    ti.screenPosition = glm::vec2(infoTextX + 1.f, infoTextY + 1.f);
    render->text.renderText(ti);

    ti.baseColour = ui_timeColour;
    ti.screenPosition = glm::vec2(infoTextX, infoTextY);
    render->text.renderText(ti);

    infoTextY += ui_textHeight;

    {
        std::stringstream ss;
        ss << std::setw(8) << std::setfill('0')
           << world->state->playerInfo.displayedMoney;

        ti.text = GameSymbols::Money + GameStringUtil::fromString(ss.str());
    }
    ti.baseColour = ui_shadowColour;
    ti.screenPosition = glm::vec2(infoTextX + 1.f, infoTextY + 1.f);
    render->text.renderText(ti);

    ti.baseColour = ui_moneyColour;
    ti.screenPosition = glm::vec2(infoTextX, infoTextY);
    render->text.renderText(ti);

    infoTextY += ui_textHeight;

    {
        std::stringstream ss;
        ss << std::setw(3) << std::setfill('0')
           << (int)player->getCharacter()->getCurrentState().health;
        ti.text = GameSymbols::Heart + GameStringUtil::fromString(ss.str());
    }
    ti.baseColour = ui_shadowColour;
    ti.screenPosition = glm::vec2(infoTextX + 1.f, infoTextY + 1.f);
    render->text.renderText(ti);

    ti.baseColour = ui_healthColour;
    ti.screenPosition = glm::vec2(infoTextX, infoTextY);
    render->text.renderText(ti);

    if (player->getCharacter()->getCurrentState().armour > 0) {
        std::stringstream ss;
        ss << std::setw(3) << std::setfill('0')
           << (int)player->getCharacter()->getCurrentState().armour;
        ti.text = GameSymbols::Armour + GameStringUtil::fromString(ss.str());
        ti.baseColour = ui_shadowColour;
        ti.screenPosition =
            glm::vec2(infoTextX + 1.f - ui_armourOffset, infoTextY + 1.f);
        render->text.renderText(ti);

        ti.baseColour = ui_armourColour;
        ti.screenPosition = glm::vec2(infoTextX - ui_armourOffset, infoTextY);
        render->text.renderText(ti);
    }

    GameString s;
    for (size_t i = 0; i < ui_maxWantedLevel; ++i) {
        s += GameSymbols::Star;
    }
    ti.text = s;
    ti.baseColour = ui_shadowColour;
    ti.screenPosition = glm::vec2(wantedX + 1.f, wantedY + 1.f);
    render->text.renderText(ti);

#if 0  // Useful for debugging
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

    auto item = player->getCharacter()->getActiveItem();
    auto weapon = world->data->weaponData[item];
    std::string itemTextureName = "fist";
    if (weapon && weapon->modelID > 0) {
        auto model =
            world->data->findModelInfo<SimpleModelInfo>(weapon->modelID);
        if (model != nullptr) {
            itemTextureName = model->name;
        }
    }
    // Urgh
    if (itemTextureName == "colt45") {
        itemTextureName = "pistol";
    }

    TextureData::Handle itemTexture =
        render->getData()->findSlotTexture("hud", itemTextureName);
    RW_CHECK(itemTexture != nullptr, "Item has 0 texture");
    if (itemTexture != nullptr) {
        RW_CHECK(itemTexture->getName() != 0, "Item has 0 texture");
        render->drawTexture(
            itemTexture.get(),
            glm::vec4(iconX, iconY, ui_weaponSize, ui_weaponSize));
    }

    if (weapon->fireType != WeaponData::MELEE) {
        const CharacterState& cs = player->getCharacter()->getCurrentState();
        const CharacterWeaponSlot& slotInfo = cs.weapons[cs.currentWeapon];
        ti.text = GameStringUtil::fromString(
            std::to_string(slotInfo.bulletsClip) + "-" +
            std::to_string(slotInfo.bulletsTotal));

        ti.baseColour = ui_shadowColour;
        ti.font = 2;
        ti.size = ui_ammoSize;
        ti.align = TextRenderer::TextInfo::Center;
        ti.screenPosition = glm::vec2(iconX + ui_weaponSize / 2.f,
                                      iconY + ui_weaponSize - ui_ammoHeight);
        render->text.renderText(ti);
    }
}

void drawHUD(ViewCamera& currentView, PlayerController* player,
             GameWorld* world, GameRenderer* render) {
    if (player && player->getCharacter()) {
        drawMap(currentView, player, world, render);
        drawPlayerInfo(player, world, render);
    }
}

void drawOnScreenText(GameWorld* world, GameRenderer* renderer) {
    const auto vp = glm::vec2(renderer->getRenderer()->getViewport());

    TextRenderer::TextInfo ti;
    ti.font = 2;
    ti.screenPosition = glm::vec2(10.f, 10.f);
    ti.size = 20.f;

    auto& alltext = world->state->text.getAllText();

    for (auto& l : alltext) {
        for (auto& t : l) {
            ti.size = t.size;
            ti.font = t.font;
            ti.text = t.text;
            ti.wrapX = t.wrapX;
            ti.screenPosition = (t.position / glm::vec2(640.f, 480.f)) * vp;
            switch (t.alignment) {
                case 0:
                    ti.align = TextRenderer::TextInfo::Left;
                    break;
                case 1:
                    ti.align = TextRenderer::TextInfo::Center;
                    break;
                case 2:
                    ti.align = TextRenderer::TextInfo::Right;
                    break;
            }

            // Check for the background type
            if (t.colourBG.a == 0) {
                glm::vec2 shadowPosition((int8_t)t.colourBG.x,
                                         (int8_t)t.colourBG.y);

                ti.baseColour = glm::vec3(0.f);
                ti.screenPosition += shadowPosition;
                ti.backgroundColour = {0, 0, 0, 0};

                renderer->text.renderText(ti, true);

                ti.screenPosition -= shadowPosition;
            } else if (t.colourBG.a > 0) {
                ti.backgroundColour = t.colourBG;
            }

            ti.baseColour = t.colourFG;
            renderer->text.renderText(ti);
        }
    }
}
