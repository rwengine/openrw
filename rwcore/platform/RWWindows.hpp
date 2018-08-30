#ifndef _RWCORE_PLATFORM_RWWINDOWS_HPP_
#define _RWCORE_PLATFORM_RWWINDOWS_HPP_

#include <string>

/**
 * @brief Convert a wide string to a ACP string (=active code page)
 * This function converts to the active code page instead of utf8
 * because Windows functions need to understand the encoding.
 * @param str The wide string to convert
 */
std::string wideStringToACP(const wchar_t* str);

#endif
