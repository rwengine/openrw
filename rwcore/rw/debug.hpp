#ifndef _LIBRW_DEBUG_HPP_
#define _LIBRW_DEBUG_HPP_

#ifdef RW_DEBUG
#include <cstdlib>
#include <functional>

extern std::function<void()> _rw_abort_cb[2];
#define SET_RW_ABORT_CB(cb0, cb1)                                              \
    do {                                                                       \
        _rw_abort_cb[0] = cb0;                                                 \
        _rw_abort_cb[1] = cb1;                                                 \
    } while (0)

#define RW_ABORT()                                                             \
    do {                                                                       \
        if (_rw_abort_cb[0])                                                   \
            _rw_abort_cb[0]();                                                 \
        ::abort();                                                             \
    } while (0)
#define RW_ASSERT(cond)                                                        \
    do {                                                                       \
        if (!(cond))                                                           \
            RW_ABORT();                                                        \
    } while (0)

#if defined(RW_WINDOWS)
void WinBreak();
#define RW_BREAKPOINT() WinBreak()
#else
#include <csignal>
#define RW_BREAKPOINT()                                                        \
    do {                                                                       \
        if (_rw_abort_cb[0])                                                   \
            _rw_abort_cb[0]();                                                 \
        ::raise(SIGTRAP);                                                      \
        if (_rw_abort_cb[1])                                                   \
            _rw_abort_cb[1]();                                                 \
    } while (0)
#endif

#define RW_FAILED_NO_ACTION 0
#define RW_FAILED_ABORT_OPTION 1
#define RW_FAILED_BREAKPOINT_OPTION 2

#if RW_FAILED_CHECK_ACTION == RW_FAILED_NO_ACTION
#define _RW_FAILED_CHECK_ACTION()
#elif RW_FAILED_CHECK_ACTION == RW_FAILED_ABORT_OPTION
#define _RW_FAILED_CHECK_ACTION() RW_ABORT()
#elif RW_FAILED_CHECK_ACTION == RW_FAILED_BREAKPOINT_OPTION
#define _RW_FAILED_CHECK_ACTION() RW_BREAKPOINT()
#endif

#else
#define SET_RW_ABORT_CB(cb0, cb1)
#define RW_ABORT()
#define RW_ASSERT(cond)
#define RW_BREAKPOINT()
#define _RW_FAILED_CHECK_ACTION()
#endif

#if defined(RW_DEBUG) && defined(RW_VERBOSE_DEBUG_MESSAGES)
#include <iostream>
#define RW_MESSAGE(msg)                                                        \
    std::cout << __FILE__ << ":" << __LINE__ << ": " << msg << std::endl
#define RW_ERROR(msg)                                                          \
    std::cerr << __FILE__ << ":" << __LINE__ << ": " << msg << std::endl
#else
#define RW_MESSAGE(msg)
#define RW_ERROR(msg)
#endif

#if RW_DEBUG
#define RW_CHECK(cond, msg)                                                    \
    do {                                                                       \
        if (!(cond)) {                                                         \
            RW_ERROR(msg);                                                     \
            _RW_FAILED_CHECK_ACTION();                                         \
        }                                                                      \
    } while (0)
#else
#define RW_CHECK(cond, msg)
#endif

#define RW_UNUSED(var) (void) var

#define RW_UNIMPLEMENTED(msg) RW_MESSAGE("Unimplemented: " << msg)

#endif
