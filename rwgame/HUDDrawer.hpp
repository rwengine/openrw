#ifndef _RWGAME_HUDDRAWER_HPP_
#define _RWGAME_HUDDRAWER_HPP_

namespace ai {
class PlayerController;
}  // namespace ai

class GameWorld;
class GameRenderer;
class ViewCamera;

#include <glm/gtc/type_precision.hpp>
#include <glm/vec3.hpp>

class HUDDrawer {
public:
    struct HUDParameters {
        float hudScale = 1.f;
        float uiTextSize = 25.f;
        float uiTextHeight = 22.f;
        float uiElementMargin = 3.f;
        float uiOuterMargin = 20.f;
        float uiInfoMargin = 10.f;
        float uiWeaponSize = 64.f;
        float uiAmmoSize = 14.f;
        float uiAmmoHeight = 16.f;
        float uiWantedLevelHeight =
            uiOuterMargin + uiWeaponSize + uiElementMargin;
        float uiScriptTimerHeight =
            uiWantedLevelHeight + uiTextHeight + uiElementMargin;
        float uiArmourOffset = uiTextSize * 3.f;
        float uiMaxWantedLevel = 6.f;
        float uiLowHealth = 9.f;
        glm::u8vec3 uiTimeColour = glm::u8vec3(196, 165, 119);
        glm::u8vec3 uiMoneyColour = glm::u8vec3(89, 113, 147);
        glm::u8vec3 uiHealthColour = glm::u8vec3(187, 102, 47);
        glm::u8vec3 uiArmourColour = glm::u8vec3(123, 136, 93);
        glm::u8vec3 uiScriptTimerColour = glm::u8vec3(186, 101, 50);
        glm::u8vec3 uiShadowColour = glm::u8vec3(0, 0, 0);
        glm::u8vec3 uiRampageTimerColour = glm::u8vec3(123, 136, 93);
        float uiMapSize = 150.f;
        float uiWorldSizeMin = 200.f;
        float uiWorldSizeMax = 300.f;
    };

    HUDDrawer() = default;
    ~HUDDrawer() = default;

    void applyHUDScale(float scale);
    HUDParameters getHUDParameters();
    void drawHUD(ViewCamera& currentView, ai::PlayerController* player,
                 GameWorld* world, GameRenderer& render);
    void drawOnScreenText(GameWorld* world, GameRenderer& renderer);

private:
    HUDParameters hudParameters;

    void drawScriptTimer(GameWorld* world, GameRenderer& render);
    void drawMap(ViewCamera& currentView, ai::PlayerController* player,
                 GameWorld* world, GameRenderer& render);
    void drawPlayerInfo(ai::PlayerController* player, GameWorld* world,
                        GameRenderer& render);
    void drawRampage(GameWorld* world, GameRenderer& render);
};

#endif
