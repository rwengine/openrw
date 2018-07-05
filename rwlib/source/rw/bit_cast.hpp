#ifndef _LIBRW_BIT_CAST_HPP_
#define _LIBRW_BIT_CAST_HPP_

//Based on https://gist.github.com/socantre/3472964
#include <cstring> // memcpy
#include <type_traits> // is_trivially_copyable

template <class Dest, class Source>
inline Dest bit_cast(Source const &source) {
    Dest dest = Dest{};
    std::memcpy(&dest, &source, sizeof(Dest));
    return dest;
}

#endif
