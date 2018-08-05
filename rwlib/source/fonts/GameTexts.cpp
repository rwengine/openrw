#include "GameTexts.hpp"

// FIXME: Update for GTA VC
#include "FontMapGta3.hpp"

#include "rw/debug.hpp"

GameString GameStringUtil::fromString(const std::string& str, font_t font) {
    RW_ASSERT(font < FONTS_COUNT);
    return fontmaps_gta3_font[font].to_GameString(str);
}

GameString GameStringUtil::fromStringCommon(const std::string& str) {
    return fontmap_gta3_font_common.to_GameString(str);
}

std::string GameStringUtil::toString(const GameString& str, font_t font) {
    return fontmaps_gta3_font[font].to_string(str);
}
