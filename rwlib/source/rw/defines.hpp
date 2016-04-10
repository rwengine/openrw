#ifndef _LIBRW_DEFINES_HPP_
#define _LIBRW_DEFINES_HPP_

#include <iostream>

#define RW_UNIMPLEMENTED(msg) \
	std::cout << "Unimplemented: " << msg << std::endl;

#if RW_DEBUG
#define RW_CHECK(cond, msg) \
	if(!(cond)) std::cerr << "Check Failed: " << msg << std::endl
#else
#define RW_CHECK(cond, msg)
#endif


#endif
