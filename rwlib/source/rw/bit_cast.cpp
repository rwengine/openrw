#ifndef _LIBRW_BIT_CAST_CPP_
#define _LIBRW_BIT_CAST_CPP_

//Based on https://gist.github.com/socantre/3472964
#include <cstring> // memcpy
#include "rw/defines.hpp" // RW_ASSERT

template <class Dest, class Source>
inline Dest bit_cast(Source const &source) {
    Dest dest = Dest{};
    std::memcpy(&dest, &source, sizeof(Dest));
    return dest;
}

#endif
