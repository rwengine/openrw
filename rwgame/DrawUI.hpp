#ifndef _RWGAME_DRAWUI_HPP_
#define _RWGAME_DRAWUI_HPP_
#include <engine/GameWorld.hpp>
#include <render/GameRenderer.hpp>

class PlayerController;

/**
 * Wrapper class for HUD drawing parameters.
 */
class HUDParameters {
public:
    struct Parameters {
        float hudScale;
        float uiTextSize;
        float uiTextHeight;
        float uiElementMargin;
        float uiOuterMargin;
        float uiInfoMargin;
        float uiWeaponSize;
        float uiAmmoSize;
        float uiAmmoHeight;
        float uiWantedLevelHeight;
        float uiScriptTimerHeight;
        float uiArmourOffset;
        float uiMapSize;
    };

    static const size_t uiTextSize = 25;
    static const size_t uiTextHeight = 22;
    static const size_t uiElementMargin = 3;
    static const size_t uiOuterMargin = 20;
    static const size_t uiInfoMargin = 10;
    static const size_t uiWeaponSize = 64;
    static const size_t uiAmmoSize = 14;
    static const size_t uiAmmoHeight = 16;
    static const size_t uiWantedLevelHeight =
        uiOuterMargin + uiWeaponSize + uiElementMargin;
    static const size_t uiScriptTimerHeight =
        uiWantedLevelHeight + uiTextHeight + uiElementMargin;
    static const size_t uiArmourOffset = uiTextSize * 3;
    static const size_t uiMaxWantedLevel = 6;
    static const size_t uiLowHealth = 9;
    static constexpr glm::u8vec3 uiTimeColour = glm::u8vec3(196, 165, 119);
    static constexpr glm::u8vec3 uiMoneyColour = glm::u8vec3(89, 113, 147);
    static constexpr glm::u8vec3 uiHealthColour = glm::u8vec3(187, 102, 47);
    static constexpr glm::u8vec3 uiArmourColour = glm::u8vec3(123, 136, 93);
    static constexpr glm::u8vec3 uiScriptTimerColour = glm::u8vec3(186, 101, 50);
    static constexpr glm::u8vec3 uiShadowColour = glm::u8vec3(0, 0, 0);
    static constexpr float uiMapSize = 150.f;
    static constexpr float uiWorldSizeMin = 200.f;
    static constexpr float uiWorldSizeMax = 300.f;

    static HUDParameters& getInstance() {
        static HUDParameters instance;

        return instance;
    }
    
    void scaleHUD(float scale = 1.f);
    Parameters getHUDParameters();

private:
    Parameters hudParameters;

    HUDParameters() = default;
    ~HUDParameters() = default;
    HUDParameters(const HUDParameters&) = delete;
    HUDParameters& operator=(const HUDParameters&) = delete;
};

void drawHUD(ViewCamera& currentView, PlayerController* player,
             GameWorld* world, GameRenderer* render);

void drawOnScreenText(GameWorld* world, GameRenderer* renderer);

#endif
