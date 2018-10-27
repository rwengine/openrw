#include "DrawUI.hpp"
#include <ai/PlayerController.hpp>
#include <data/WeaponData.hpp>
#include <engine/GameData.hpp>
#include <engine/GameState.hpp>
#include <objects/CharacterObject.hpp>
#include <render/GameRenderer.hpp>

#include <glm/gtc/constants.hpp>
#include <iomanip>
#include <sstream>

void drawScriptTimer(GameWorld* world, GameRenderer* render) {
    HUDParameters& hudParams = HUDParameters::getInstance();
    HUDParameters::Parameters parameters = hudParams.getHUDParameters();

    if (world->state->scriptTimerVariable) {
        float scriptTimerTextX = static_cast<float>(
            render->getRenderer()->getViewport().x - parameters.uiOuterMargin);
        float scriptTimerTextY = parameters.uiScriptTimerHeight;

        TextRenderer::TextInfo ti;
        ti.font = FONT_PRICEDOWN;
        ti.size = parameters.uiTextSize;
        ti.align = TextRenderer::TextInfo::TextAlignment::Right;

        {
            int32_t seconds = *world->state->scriptTimerVariable / 1000;
            std::stringstream ss;
            ss << std::setw(2) << std::setfill('0') << seconds / 60
               << std::setw(0) << ":" << std::setw(2) << seconds % 60;

            ti.text = GameStringUtil::fromString(ss.str(), ti.font);
        }

        ti.baseColour = hudParams.uiShadowColour;
        ti.screenPosition =
            glm::vec2(scriptTimerTextX + 1.f, scriptTimerTextY + 1.f);
        render->text.renderText(ti);

        ti.baseColour = hudParams.uiScriptTimerColour;
        ti.screenPosition = glm::vec2(scriptTimerTextX, scriptTimerTextY);
        render->text.renderText(ti);
    }
}

void drawMap(ViewCamera& currentView, PlayerController* player,
             GameWorld* world, GameRenderer* render) {
    MapRenderer::MapInfo map;
    HUDParameters& hudParams = HUDParameters::getInstance();
    HUDParameters::Parameters parameters = hudParams.getHUDParameters();

    if (world->state->hudFlash != HudFlash::FlashRadar ||
        std::fmod(world->getGameTime(), 0.5f) >= .25f) {
        glm::quat camRot = currentView.rotation;

        map.rotation = glm::roll(camRot) - glm::half_pi<float>();
        map.worldSize = hudParams.uiWorldSizeMin;
        map.worldSize = hudParams.uiWorldSizeMax;
        if (player) {
            map.worldCenter = glm::vec2(player->getCharacter()->getPosition());
        }

        const glm::ivec2& vp = render->getRenderer()->getViewport();

        glm::vec2 mapTop =
            glm::vec2(parameters.uiOuterMargin, vp.y - (parameters.uiOuterMargin + parameters.uiMapSize));
        glm::vec2 mapBottom =
            glm::vec2(parameters.uiOuterMargin + parameters.uiMapSize, vp.y - parameters.uiOuterMargin);

        map.screenPosition = (mapTop + mapBottom) / 2.f;
        map.screenSize = parameters.uiMapSize * 0.95f;

        render->map.draw(world, map);
    }
}

void drawPlayerInfo(PlayerController* player, GameWorld* world,
                    GameRenderer* render) {
    HUDParameters& hudParams = HUDParameters::getInstance();
    HUDParameters::Parameters parameters = hudParams.getHUDParameters();

    float infoTextX = static_cast<float>(render->getRenderer()->getViewport().x -
                      (parameters.uiOuterMargin + parameters.uiWeaponSize + parameters.uiInfoMargin));
    float infoTextY = 0.f + parameters.uiOuterMargin;
    float iconX = static_cast<float>(render->getRenderer()->getViewport().x -
                  (parameters.uiOuterMargin + parameters.uiWeaponSize));
    float iconY = parameters.uiOuterMargin;
    float wantedX = static_cast<float>(render->getRenderer()->getViewport().x - parameters.uiOuterMargin);
    float wantedY = parameters.uiWantedLevelHeight;

    TextRenderer::TextInfo ti;
    ti.font = FONT_PRICEDOWN;
    ti.size = parameters.uiTextSize;
    ti.align = TextRenderer::TextInfo::TextAlignment::Right;

    {
        std::stringstream ss;
        ss << std::setw(2) << std::setfill('0') << world->getHour()
           << std::setw(0) << ":" << std::setw(2) << world->getMinute();

        ti.text = GameStringUtil::fromString(ss.str(), ti.font);
    }

    ti.baseColour = hudParams.uiShadowColour;
    ti.screenPosition = glm::vec2(infoTextX + 1.f, infoTextY + 1.f);
    render->text.renderText(ti);

    ti.baseColour = hudParams.uiTimeColour;
    ti.screenPosition = glm::vec2(infoTextX, infoTextY);

    render->text.renderText(ti);

    infoTextY += parameters.uiTextHeight;

    {
        std::stringstream ss;
        ss << std::setw(8) << std::setfill('0')
           << world->state->playerInfo.displayedMoney;

        ti.text = GameSymbols::Money + GameStringUtil::fromString(ss.str(), ti.font);
    }

    ti.baseColour = hudParams.uiShadowColour;
    ti.screenPosition = glm::vec2(infoTextX + 1.f, infoTextY + 1.f);
    render->text.renderText(ti);

    ti.baseColour = hudParams.uiMoneyColour;

    ti.screenPosition = glm::vec2(infoTextX, infoTextY);
    render->text.renderText(ti);

    infoTextY += parameters.uiTextHeight;

    if ((world->state->hudFlash != HudFlash::FlashHealth &&
         player->getCharacter()->getCurrentState().health > hudParams.uiLowHealth) ||
        std::fmod(world->getGameTime(), 0.5f) >=
            .25f) {  // UI: Blinking health indicator if health is low
        std::stringstream ss;
        ss << std::setw(3) << std::setfill('0')
           << static_cast<int>(
                  player->getCharacter()->getCurrentState().health);
        ti.text = GameSymbols::Heart + GameStringUtil::fromString(ss.str(), ti.font);

        ti.baseColour = hudParams.uiShadowColour;
        ti.screenPosition = glm::vec2(infoTextX + 1.f, infoTextY + 1.f);

        render->text.renderText(ti);

        ti.baseColour = hudParams.uiHealthColour;
        ti.screenPosition = glm::vec2(infoTextX, infoTextY);
        render->text.renderText(ti);
    }

    if (player->getCharacter()->getCurrentState().armour > 0) {
        std::stringstream ss;
        ss << std::setw(3) << std::setfill('0')
           << static_cast<int>(
                  player->getCharacter()->getCurrentState().armour);
        ti.text = GameSymbols::Armour + GameStringUtil::fromString(ss.str(), ti.font);

        ti.baseColour = hudParams.uiShadowColour;
        ti.screenPosition =
            glm::vec2(infoTextX + 1.f - parameters.uiArmourOffset, infoTextY + 1.f);
        render->text.renderText(ti);

        ti.baseColour = hudParams.uiArmourColour;
        ti.screenPosition = glm::vec2(infoTextX - parameters.uiArmourOffset, infoTextY);
        render->text.renderText(ti);
    }

    GameString s;
    for (size_t i = 0; i < hudParams.uiMaxWantedLevel; ++i) {
        s += GameSymbols::Star;
    }
    ti.text = s;
    ti.baseColour = hudParams.uiShadowColour;
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
            glm::vec4(iconX, iconY, parameters.uiWeaponSize, parameters.uiWeaponSize));
    }

    if (weapon->fireType != WeaponData::MELEE) {
        const CharacterState& cs = player->getCharacter()->getCurrentState();
        const CharacterWeaponSlot& slotInfo = cs.weapons[cs.currentWeapon];

        // In weapon.dat clip size of 0 or 1000+ indicates no reload
        // Clip size of 1 is being visually omitted as well
        bool noClip = weapon->clipSize < 2 || weapon->clipSize > 999;

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

        ti.baseColour = hudParams.uiShadowColour;
        ti.font = FONT_ARIAL;
        ti.size = parameters.uiAmmoSize;
        ti.align = TextRenderer::TextInfo::TextAlignment::Center;
        ti.screenPosition = glm::vec2(iconX + parameters.uiWeaponSize / 2.f,
                                      iconY + parameters.uiWeaponSize - parameters.uiAmmoHeight);
        render->text.renderText(ti);
    }
}

void drawHUD(ViewCamera& currentView, PlayerController* player,
             GameWorld* world, GameRenderer* render) {
    if (player && player->getCharacter()) {
        drawMap(currentView, player, world, render);
        drawPlayerInfo(player, world, render);
        drawScriptTimer(world, render);
    }
}

void drawOnScreenText(GameWorld* world, GameRenderer* renderer) {
    HUDParameters::Parameters parameters = HUDParameters::getInstance().getHUDParameters();

    const auto vp = glm::vec2(renderer->getRenderer()->getViewport());

    TextRenderer::TextInfo ti;
    ti.font = FONT_ARIAL;
    ti.screenPosition = glm::vec2(10.f, 10.f);
    ti.size = 20.f;

    auto& alltext = world->state->text.getAllText();

    for (auto& l : alltext) {
        for (auto& t : l) {
            ti.size = static_cast<float>(t.size * parameters.hudScale);
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

void HUDParameters::scaleHUD(float scale) {
    hudParameters.hudScale = scale;
    hudParameters.uiTextSize = uiTextSize * scale;
    hudParameters.uiTextHeight = uiTextHeight * scale;
    hudParameters.uiElementMargin = uiElementMargin * scale;
    hudParameters.uiOuterMargin = uiOuterMargin * scale;
    hudParameters.uiInfoMargin = uiInfoMargin * scale;
    hudParameters.uiWeaponSize = uiWeaponSize * scale;
    hudParameters.uiAmmoSize = uiAmmoSize * scale;
    hudParameters.uiAmmoHeight = uiAmmoHeight * scale;
    hudParameters.uiWantedLevelHeight = uiWantedLevelHeight * scale;
    hudParameters.uiScriptTimerHeight = uiScriptTimerHeight * scale;
    hudParameters.uiArmourOffset = uiArmourOffset * scale;
    hudParameters.uiMapSize = uiMapSize * scale;
}

HUDParameters::Parameters HUDParameters::getHUDParameters() {
    return hudParameters;
}
