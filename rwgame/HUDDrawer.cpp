#include "HUDDrawer.hpp"
#include <ai/PlayerController.hpp>
#include <data/WeaponData.hpp>
#include <engine/GameData.hpp>
#include <engine/GameState.hpp>
#include <objects/CharacterObject.hpp>
#include <render/GameRenderer.hpp>

#include <glm/gtc/constants.hpp>
#include <iomanip>
#include <sstream>

void HUDDrawer::drawScriptTimer(GameWorld* world, GameRenderer* render) {
    if (world->state->scriptTimerVariable) {
        float scriptTimerTextX = static_cast<float>(
            render->getRenderer()->getViewport().x - hudParameters.uiOuterMargin);
        float scriptTimerTextY = hudParameters.uiScriptTimerHeight;

        TextRenderer::TextInfo ti;
        ti.font = FONT_PRICEDOWN;
        ti.size = hudParameters.uiTextSize;
        ti.align = TextRenderer::TextInfo::TextAlignment::Right;

        {
            int32_t seconds = *world->state->scriptTimerVariable / 1000;
            std::stringstream ss;
            ss << std::setw(2) << std::setfill('0') << seconds / 60
               << std::setw(0) << ":" << std::setw(2) << seconds % 60;

            ti.text = GameStringUtil::fromString(ss.str(), ti.font);
        }

        ti.baseColour = hudParameters.uiShadowColour;
        ti.screenPosition =
            glm::vec2(scriptTimerTextX + 1.f, scriptTimerTextY + 1.f);
        render->text.renderText(ti);

        ti.baseColour = hudParameters.uiScriptTimerColour;
        ti.screenPosition = glm::vec2(scriptTimerTextX, scriptTimerTextY);
        render->text.renderText(ti);
    }
}

void HUDDrawer::drawMap(ViewCamera& currentView, PlayerController* player,
             GameWorld* world, GameRenderer* render) {
    MapRenderer::MapInfo map;

    if (world->state->hudFlash != HudFlash::FlashRadar ||
        std::fmod(world->getGameTime(), 0.5f) >= .25f) {
        glm::quat camRot = currentView.rotation;

        map.rotation = glm::roll(camRot) - glm::half_pi<float>();
        map.worldSize = hudParameters.uiWorldSizeMin;
        map.worldSize = hudParameters.uiWorldSizeMax;
        if (player) {
            map.worldCenter = glm::vec2(player->getCharacter()->getPosition());
        }

        const glm::ivec2& vp = render->getRenderer()->getViewport();

        glm::vec2 mapTop =
            glm::vec2(hudParameters.uiOuterMargin, vp.y - (hudParameters.uiOuterMargin + hudParameters.uiMapSize));
        glm::vec2 mapBottom =
            glm::vec2(hudParameters.uiOuterMargin + hudParameters.uiMapSize, vp.y - hudParameters.uiOuterMargin);

        map.screenPosition = (mapTop + mapBottom) / 2.f;
        map.screenSize = hudParameters.uiMapSize * 0.95f;

        render->map.draw(world, map);
    }
}

void HUDDrawer::drawPlayerInfo(PlayerController* player, GameWorld* world,
                    GameRenderer* render) {
    float infoTextX = static_cast<float>(render->getRenderer()->getViewport().x -
                      (hudParameters.uiOuterMargin + hudParameters.uiWeaponSize + hudParameters.uiInfoMargin));
    float infoTextY = 0.f + hudParameters.uiOuterMargin;
    float iconX = static_cast<float>(render->getRenderer()->getViewport().x -
                  (hudParameters.uiOuterMargin + hudParameters.uiWeaponSize));
    float iconY = hudParameters.uiOuterMargin;
    float wantedX = static_cast<float>(render->getRenderer()->getViewport().x - hudParameters.uiOuterMargin);
    float wantedY = hudParameters.uiWantedLevelHeight;

    TextRenderer::TextInfo ti;
    ti.font = FONT_PRICEDOWN;
    ti.size = hudParameters.uiTextSize;
    ti.align = TextRenderer::TextInfo::TextAlignment::Right;

    {
        std::stringstream ss;
        ss << std::setw(2) << std::setfill('0') << world->getHour()
           << std::setw(0) << ":" << std::setw(2) << world->getMinute();

        ti.text = GameStringUtil::fromString(ss.str(), ti.font);
    }

    ti.baseColour = hudParameters.uiShadowColour;
    ti.screenPosition = glm::vec2(infoTextX + 1.f, infoTextY + 1.f);
    render->text.renderText(ti);

    ti.baseColour = hudParameters.uiTimeColour;
    ti.screenPosition = glm::vec2(infoTextX, infoTextY);

    render->text.renderText(ti);

    infoTextY += hudParameters.uiTextHeight;

    {
        std::stringstream ss;
        ss << std::setw(8) << std::setfill('0')
           << world->state->playerInfo.displayedMoney;

        ti.text = GameSymbols::Money + GameStringUtil::fromString(ss.str(), ti.font);
    }

    ti.baseColour = hudParameters.uiShadowColour;
    ti.screenPosition = glm::vec2(infoTextX + 1.f, infoTextY + 1.f);
    render->text.renderText(ti);

    ti.baseColour = hudParameters.uiMoneyColour;

    ti.screenPosition = glm::vec2(infoTextX, infoTextY);
    render->text.renderText(ti);

    infoTextY += hudParameters.uiTextHeight;

    if ((world->state->hudFlash != HudFlash::FlashHealth &&
         player->getCharacter()->getCurrentState().health > hudParameters.uiLowHealth) ||
        std::fmod(world->getGameTime(), 0.5f) >=
            .25f) {  // UI: Blinking health indicator if health is low
        std::stringstream ss;
        ss << std::setw(3) << std::setfill('0')
           << static_cast<int>(
                  player->getCharacter()->getCurrentState().health);
        ti.text = GameSymbols::Heart + GameStringUtil::fromString(ss.str(), ti.font);

        ti.baseColour = hudParameters.uiShadowColour;
        ti.screenPosition = glm::vec2(infoTextX + 1.f, infoTextY + 1.f);

        render->text.renderText(ti);

        ti.baseColour = hudParameters.uiHealthColour;
        ti.screenPosition = glm::vec2(infoTextX, infoTextY);
        render->text.renderText(ti);
    }

    if (player->getCharacter()->getCurrentState().armour > 0) {
        std::stringstream ss;
        ss << std::setw(3) << std::setfill('0')
           << static_cast<int>(
                  player->getCharacter()->getCurrentState().armour);
        ti.text = GameSymbols::Armour + GameStringUtil::fromString(ss.str(), ti.font);

        ti.baseColour = hudParameters.uiShadowColour;
        ti.screenPosition =
            glm::vec2(infoTextX + 1.f - hudParameters.uiArmourOffset, infoTextY + 1.f);
        render->text.renderText(ti);

        ti.baseColour = hudParameters.uiArmourColour;
        ti.screenPosition = glm::vec2(infoTextX - hudParameters.uiArmourOffset, infoTextY);
        render->text.renderText(ti);
    }

    GameString s;
    for (size_t i = 0; i < hudParameters.uiMaxWantedLevel; ++i) {
        s += GameSymbols::Star;
    }
    ti.text = s;
    ti.baseColour = hudParameters.uiShadowColour;
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
    const auto& weapon = world->data->weaponData[item];
    std::string itemTextureName = "fist";
    if (weapon.modelID > 0) {
        auto model =
            world->data->findModelInfo<SimpleModelInfo>(weapon.modelID);
        if (model != nullptr) {
            itemTextureName = model->name;
        }
    }
    // Urgh
    if (itemTextureName == "colt45") {
        itemTextureName = "pistol";
    } else if (itemTextureName == "bomb") {
        itemTextureName = "detonator";
    }

    TextureData::Handle itemTexture =
        render->getData()->findSlotTexture("hud", itemTextureName);
    RW_CHECK(itemTexture != nullptr, "Item has 0 texture");
    if (itemTexture != nullptr) {
        RW_CHECK(itemTexture->getName() != 0, "Item has 0 texture");
        render->drawTexture(
            itemTexture.get(),
            glm::vec4(iconX, iconY, hudParameters.uiWeaponSize, hudParameters.uiWeaponSize));
    }

    if (weapon.fireType != WeaponData::MELEE) {
        const CharacterState& cs = player->getCharacter()->getCurrentState();
        const CharacterWeaponSlot& slotInfo = cs.weapons[cs.currentWeapon];

        // In weapon.dat clip size of 0 or 1000+ indicates no reload
        // Clip size of 1 is being visually omitted as well
        bool noClip = weapon.clipSize < 2 || weapon.clipSize > 999;

        uint32_t displayBulletsTotal = slotInfo.bulletsTotal;

        if (noClip) {
            // The clip is actually there, but it holds just one shot/charge
            displayBulletsTotal += slotInfo.bulletsClip;

            ti.text =
                GameStringUtil::fromString(std::to_string(displayBulletsTotal), ti.font);
        } else {
            // Limit the maximal displayed length for the total bullet count
            if (slotInfo.bulletsTotal > 9999) {
                displayBulletsTotal = 9999;
            }

            ti.text = GameStringUtil::fromString(
                std::to_string(displayBulletsTotal) + "-" +
                std::to_string(slotInfo.bulletsClip), ti.font);
        }

        ti.baseColour = hudParameters.uiShadowColour;
        ti.font = FONT_ARIAL;
        ti.size = hudParameters.uiAmmoSize;
        ti.align = TextRenderer::TextInfo::TextAlignment::Center;
        ti.screenPosition = glm::vec2(iconX + hudParameters.uiWeaponSize / 2.f,
                                      iconY + hudParameters.uiWeaponSize - hudParameters.uiAmmoHeight);
        render->text.renderText(ti);
    }
}

void HUDDrawer::drawHUD(ViewCamera& currentView, PlayerController* player,
             GameWorld* world, GameRenderer* render) {
    if (player && player->getCharacter()) {
        drawMap(currentView, player, world, render);
        drawPlayerInfo(player, world, render);
        drawScriptTimer(world, render);
    }
}

void HUDDrawer::drawOnScreenText(GameWorld* world, GameRenderer* renderer) {
    const auto vp = glm::vec2(renderer->getRenderer()->getViewport());

    TextRenderer::TextInfo ti;
    ti.font = FONT_ARIAL;
    ti.screenPosition = glm::vec2(10.f, 10.f);
    ti.size = 20.f;

    auto& alltext = world->state->text.getAllText();

    for (auto& l : alltext) {
        for (auto& t : l) {
            ti.size = static_cast<float>(t.size * hudParameters.hudScale);
            ti.font = t.font;
            ti.text = t.text;
            ti.wrapX = t.wrapX;
            ti.screenPosition = (t.position / glm::vec2(640.f, 480.f)) * vp;
            switch (t.alignment) {
                case 0:
                    ti.align = TextRenderer::TextInfo::TextAlignment::Left;
                    break;
                case 1:
                    ti.align = TextRenderer::TextInfo::TextAlignment::Center;
                    break;
                case 2:
                    ti.align = TextRenderer::TextInfo::TextAlignment::Right;
                    break;
            }

            // Check for the background type
            if (t.colourBG.a == 0) {
                glm::vec2 shadowPosition(static_cast<int8_t>(t.colourBG.x),
                                         static_cast<int8_t>(t.colourBG.y));

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

void HUDDrawer::applyHUDScale(float scale) {
    hudParameters.hudScale = scale;
    hudParameters.uiTextSize *= scale;
    hudParameters.uiTextHeight *= scale;
    hudParameters.uiElementMargin *= scale;
    hudParameters.uiOuterMargin *= scale;
    hudParameters.uiInfoMargin *= scale;
    hudParameters.uiWeaponSize *= scale;
    hudParameters.uiAmmoSize *= scale;
    hudParameters.uiAmmoHeight *= scale;
    hudParameters.uiWantedLevelHeight *= scale;
    hudParameters.uiScriptTimerHeight *= scale;
    hudParameters.uiArmourOffset *= scale;
    hudParameters.uiMapSize *= scale;
}

HUDDrawer::HUDParameters HUDDrawer::getHUDParameters() {
    return hudParameters;
}
