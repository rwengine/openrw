#ifndef _LIBRW_CASTS_HPP_
#define _LIBRW_CASTS_HPP_

#include <cstring> // memcpy

#include "rw/debug.hpp"

//Based on https://gist.github.com/socantre/3472964
template <class Dest, class Source>
inline Dest bit_cast(Source const &source) {
    Dest dest = Dest{};
    std::memcpy(&dest, &source, sizeof(Dest));
    return dest;
}

template <class T, class S>
inline T lexical_cast(const S& s);

template <class T, class S>
inline T lexical_cast(const S& s, size_t base);

template <>
inline int lexical_cast(const std::string& source, size_t base) {
    char* end = nullptr; //for errors handling
    int result = std::strtol(source.c_str(), &end, base);
    RW_CHECK(end != source.c_str(), "Problem with conversion " << *end << " to int");
    return result;
}

template <>
inline int lexical_cast(const std::string& source) {
    return lexical_cast<int>(source, 10);
}

template <>
inline float lexical_cast(const std::string& source) {
    char* end = nullptr; //for errors handling
    float result = std::strtof(source.c_str(), &end);
    RW_CHECK(end != source.c_str(), "Problem with conversion " << *end << " to float");
    return result;
}

#endif
