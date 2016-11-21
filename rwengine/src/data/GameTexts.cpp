#include "GameTexts.hpp"

GameString GameStringUtil::fromString(const std::string& str) {
    GameString s;
    for (std::string::size_type i = 0u; i < str.size(); ++i) {
        s += str[i];
    }
    return s;
}
