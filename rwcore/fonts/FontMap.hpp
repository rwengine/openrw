#ifndef _RWLIB_FONTS_FONTMAP_HPP_
#define _RWLIB_FONTS_FONTMAP_HPP_

#include <functional>
#include <initializer_list>
#include <map>

#include "GameTexts.hpp"
#include "Unicode.hpp"

/**
 * @brief Class providing mapping from unicode chars to game strings and vice
 * versa. The conversions of an object of this class depend on the actual font
 * used.
 * @param maps
 */
class FontMap {
public:
    /**
     * Mapping from GameStringChar to unicode data point.
     */
    using gschar_unicode_map_t = std::map<GameStringChar, unicode_t>;

    /**
     * Mapping from unicode data point to GameStringChar.
     */
    using unicode_gschar_map_t = std::map<unicode_t, GameStringChar>;

    /**
     * Iterator type over all GameStringChar to unicode.
     */
    using gschar_unicode_iterator = gschar_unicode_map_t::const_iterator;

    /**
     * @brief FontMap Create a new Fontmapping using the maps provided.
     * @param maps List of mappings used as source for this font mapping.
     */
    FontMap(std::initializer_list<
            std::reference_wrapper<const gschar_unicode_map_t>>
                maps);

    /**
     * @brief to_GameStringChar Convert a unicode data point to a
     * GameStringChar.
     * @param u The unicode character.
     * @return  A GameStringChar
     */
    GameStringChar to_GameStringChar(unicode_t u) const;

    /**
     * @brief to_unicoe Convert a GameStringChar to a unicode data point.
     * @param c The GameStringChar
     * @return A unicode character.
     */
    unicode_t to_unicode(GameStringChar c) const;

    /**
     * @brief to_string Convert a GameString to a utf-8 encoded string.
     * @param s The GameString.
     * @return A utf-8 encoded string.
     */
    std::string to_string(const GameString& s) const;

    /**
     * @brief to_GameString Convert a utf-8 encoded string to a GameString.
     * @param utf8 The utf-8 encoded string.
     * @return  A GameString.
     */
    GameString to_GameString(const std::string& utf8) const;

    /**
     * @brief to_unicode_begin Iterate over the GameStringChar to unicode begin.
     * @return Iterator to begin.
     */
    gschar_unicode_iterator to_unicode_begin() const;

    /**
     * @brief to_unicode_begin Iterate over the GameStringChar to unicode end.
     * @return Iterator Iterator to end.
     */
    gschar_unicode_iterator to_unicode_end() const;

private:
    /**
     * Mapping from a unicode point to a GameStringChar.
     */
    unicode_gschar_map_t m_from_unicode;

    /**
     * Mapping from a GameStringChar to a unicode point.
     */
    gschar_unicode_map_t m_to_unicode;

    /**
     * GameStringChar used if a unicode point has no corresponding
     * GameStringChar.
     */
    GameStringChar m_unknown_gschar;
};

#endif
