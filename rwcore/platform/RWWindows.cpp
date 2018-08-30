#include "RWWindows.hpp"

#include "rw/Debug.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

std::string wideStringToACP(const wchar_t* wString) {
    std::string returnValue;
    bool result = true;

    int nbChars = WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, wString, -1,
                                      nullptr, 0, nullptr, nullptr);
    if (nbChars == 0) {
        if (GetLastError() != ERROR_SUCCESS) {
            RW_ERROR("Unable to calculate length of wide string");
            return returnValue;
        }
    }
    returnValue.resize(nbChars);
    nbChars =
        WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, wString, -1,
                            &returnValue.front(), nbChars, nullptr, nullptr);
    if (nbChars == 0) {
        returnValue.resize(0);
        return returnValue;
    }
    returnValue.resize(nbChars - 1);
    return returnValue;
}
