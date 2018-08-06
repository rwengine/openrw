#ifndef _RWLIB_FONTS_GAMETEXTS_HPP_
#define _RWLIB_FONTS_GAMETEXTS_HPP_

#include <cstdint>
#include <string>
#include <unordered_map>
#include <sstream>
#include <string>

/**
 * Each GXT char is just a 16-bit index into the font map.
 */
using GameStringChar = std::uint16_t;

/**
 * The game stores strings as 16-bit indexes into the font
 * texture, which is something similar to ASCII.
 */
using GameString = std::basic_string<GameStringChar>;

/**
 * GXT keys are just 8 single byte chars.
 * Keys are small so should be subject to SSO
 */
using GameStringKey = std::string;

/**
 * Index to used font.
 */
using font_t = size_t;

static constexpr font_t FONT_PAGER = 0;
static constexpr font_t FONT_PRICEDOWN = 1;
static constexpr font_t FONT_ARIAL = 2;
static constexpr font_t FONTS_COUNT = 3;

namespace GameStringUtil {
/**
 * @brief fromString Converts a std::string to a GameString, depending on the font
 *
 * Encoding of GameStrings depends on the font. Unknown chars are converted to a "unknown GameStringChar" (such as '?').
 */
GameString fromString(const std::string& str, font_t font);

/**
 * @brief fromString Converts a GameString to a std::string, depending on the font
 *
 * Encoding of GameStrings depends on the font. Unknown GameStringChar's are converted to a UNICODE_REPLACEMENT_CHARACTER utf-8 sequence.
 */
std::string toString(const GameString& str, font_t font);

/**
 * @brief fromString Converts a string to a GameString, independent on the font (only characthers known to all fonts are converted)
 *
 * Encoding of GameStrings does not depend on the font. Unknown chars are converted to a "unknown GameStringChar" (such as '?').
 */
GameString fromStringCommon(const std::string& str);
}

/**
 * Since the encoding of symbols is arbitrary, these constants should be used in
 * hard-coded strings containing symbols outside of the ASCII-subset supported
 * by all fonts
 */
namespace GameSymbols {
static constexpr GameStringChar Money = '$';
static constexpr GameStringChar Heart = '{';
static constexpr GameStringChar Armour = '[';
static constexpr GameStringChar Star = ']';
}

class GameTexts {
    using StringTable = std::unordered_map<GameStringKey, GameString>;
    StringTable m_strings;

public:
    void addText(const GameStringKey& id, GameString&& text) {
        m_strings.emplace(id, text);
    }

    GameString text(const GameStringKey& id) const {
        auto a = m_strings.find(id);
        if (a != m_strings.end()) {
            return a->second;
        }
        return GameStringUtil::fromString("MISSING: " + id, FONT_ARIAL);
    }

    const StringTable &getStringTable() const {
        return m_strings;
    }
};

#endif
