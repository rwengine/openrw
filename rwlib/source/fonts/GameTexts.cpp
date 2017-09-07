#include "GameTexts.hpp"

GameString GameStringUtil::fromString(const std::string& str) {
    GameString s;
    for (const char &i : str) {
        s += i;
    }
    return s;
}
