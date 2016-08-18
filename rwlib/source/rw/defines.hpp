#ifndef _LIBRW_DEFINES_HPP_
#define _LIBRW_DEFINES_HPP_

#if RW_DEBUG && RW_VERBOSE_DEBUG_MESSAGES
#include <iostream>
#define RW_MESSAGE(msg) std::cout << __FILE__ << ":" << __LINE__ << ": " << msg << std::endl
#define RW_ERROR(msg) std::cerr << __FILE__ << ":" << __LINE__ << ": " << msg << std::endl
#define RW_CHECK(cond, msg) \
  if (!(cond)) RW_ERROR(msg)
#else
#define RW_MESSAGE(msg)
#define RW_ERROR(msg)
#define RW_CHECK(cond, msg)
#endif

#define RW_UNUSED(var) (void) var

#define RW_UNIMPLEMENTED(msg) RW_MESSAGE("Unimplemented: " << msg)

#endif
