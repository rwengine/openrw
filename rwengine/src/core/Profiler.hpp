#ifndef _RWENGINE_PROFILER_HPP_
#define _RWENGINE_PROFILER_HPP_

#ifdef RW_PROFILER
#include <microprofile.h>
#define RW_PROFILE_THREAD(name) MicroProfileOnThreadCreate(name)
#define RW_PROFILE_FRAME_BOUNDARY() MicroProfileFlip(nullptr)
#define RW_PROFILE_SCOPE(label) MICROPROFILE_SCOPEI("Default", label, MP_YELLOW)
#define RW_PROFILE_COUNTER_ADD(name, qty) MICROPROFILE_COUNTER_ADD(name, qty)
#define RW_PROFILE_COUNTER_SET(name, qty) MICROPROFILE_COUNTER_SET(name, qty)
#define RW_TIMELINE_ENTER(name, color) MICROPROFILE_TIMELINE_ENTER_STATIC(color, name)
#define RW_TIMELINE_LEAVE(name) MICROPROFILE_TIMELINE_LEAVE_STATIC(name)
#else
#define RW_PROFILE_THREAD(name) do {} while (0)
#define RW_PROFILE_FRAME_BOUNDARY() do {} while (0)
#define RW_PROFILE_SCOPE(label) do {} while (0)
#define RW_PROFILE_COUNTER_ADD(name, qty) do {} while (0)
#define RW_PROFILE_COUNTER_SET(name, qty) do {} while (0)
#define RW_TIMELINE_ENTER(name, color) do {} while (0)
#define RW_TIMELINE_LEAVE(name) do {} while (0)
#endif

#endif
