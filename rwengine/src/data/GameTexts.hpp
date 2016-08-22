#ifndef RWENGINE_GAMETEXTS_HPP
#define RWENGINE_GAMETEXTS_HPP
#include <string>
#include <unordered_map>

/**
 * Each GXT char is just a 16-bit index into the font map.
 */
using GameStringChar = uint16_t;
/**
 * The game stores strings as 16-bit indexes into the font
 * texture, which is something simllar to ASCII.
 */
using GameString = std::basic_string<GameStringChar>;
/**
 * GXT keys are just 8 single byte chars.
 * Keys are small so should be subject to SSO
 */
using GameStringKey = std::string;

namespace GameStringUtil
{
/**
 * @brief fromString Converts a string to a GameString
 *
 * Encoding of GameStrings depends on the font, only simple ASCII chars will map well
 */
GameString fromString(const std::string& str);
}

/**
 * Since the encoding of symbols is arbitrary, these constants should be used in
 * hard-coded strings containing symbols outside of the ASCII-subset supported by
 * all fonts
 */
namespace GameSymbols
{
	static constexpr GameStringChar Money  = '$';
	static constexpr GameStringChar Heart  = '{';
	static constexpr GameStringChar Armour = '[';
	static constexpr GameStringChar Star   = ']';
}

class GameTexts
{
	using StringTable = std::unordered_map<GameStringKey, GameString>;
	StringTable m_strings;
public:

	void addText(const GameStringKey& id, GameString&& text) {
		m_strings.emplace(id, text);
	}

	GameString text(const GameStringKey& id) {
		auto a = m_strings.find(id);
		if( a != m_strings.end() ) {
			return a->second;
		}
		return GameStringUtil::fromString("MISSING: " + id);
	}

};

#endif
