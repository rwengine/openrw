#include "FontMap.hpp"

#include <rw/debug.hpp>

#include <sstream>

/**
 * Use output operations to create GameStrings (only allows write operations)
 */
using OGameStringStream = std::basic_ostringstream<GameStringChar>;

FontMap::FontMap(std::initializer_list<std::reference_wrapper<const gschar_unicode_map_t>> maps) {
    for (const auto &map : maps) {
        m_to_unicode.insert(map.get().cbegin(), map.get().cend());
        for (const auto &m : map.get()) {
            m_from_unicode[m.second] = m.first;
        }
    }
    const auto &q = m_from_unicode.find(UnicodeValue::UNICODE_QUESTION_MARK);
    if (q == m_from_unicode.end()) {
        RW_ERROR("Font does not have a question mark");
        m_unknown_gschar = ' ';
    } else {
        m_unknown_gschar = q->second;
    }
}

GameStringChar FontMap::to_GameStringChar(unicode_t u) const {
    if (u < 0x20) {
        /* Passthrough control characters */
        return u;
    }
    const auto &p = m_from_unicode.find(u);
    if (p == m_from_unicode.end()) {
        return m_unknown_gschar;
    }
    return p->second;
}

unicode_t FontMap::to_unicode(GameStringChar c) const {
    if (c < 0x20) {
        /* Passthrough control characters */
        return c;
    }
    const auto &p = m_to_unicode.find(c);
    if (p == m_to_unicode.end()) {
        return UnicodeValue::UNICODE_REPLACEMENT_CHARACTER;
    }
    return p->second;
}

std::string FontMap::to_string(const GameString &s) const {
    std::ostringstream oss;
    for (GameStringChar c: s) {
        char buffer[4];
        unicode_t u = to_unicode(c);
        auto nb = unicode_to_utf8(u, buffer);
        oss.write(buffer, nb);
    }
    return oss.str();
}

GameString FontMap::to_GameString(const std::string &utf8) const {
    OGameStringStream oss;
    std::istringstream iss(utf8);
    for (Utf8UnicodeIterator it(iss); it.good(); ++it) {
        GameStringChar c = to_GameStringChar(it.unicode());
        oss.write(&c, 1);
    }
    return oss.str();
}

FontMap::gschar_unicode_iterator FontMap::to_unicode_begin() const {
    return m_to_unicode.cbegin();
}

FontMap::gschar_unicode_iterator FontMap::to_unicode_end() const {
    return m_to_unicode.cend();
}
