#ifndef _RWENGINE_ALCHECK_HPP_
#define _RWENGINE_ALCHECK_HPP_

#include <string>

void checkALerror(const std::string& file, unsigned int line);

#if RW_DEBUG
#define alCheck(stmt)                                                          \
    do {                                                                       \
        stmt;                                                                  \
        checkALerror(__FILE__, __LINE__);                                      \
    } while (0)
#else
#define alCheck(stmt) stmt
#endif

#endif
