#include <functional>

#if RW_DEBUG
std::function<void()> _rw_abort_cb[2] = {nullptr, nullptr};

#if defined(RW_WINDOWS)
#define WINDOWS_LEAN_AND_MEAN
#include <Windows.h>

void WinBreak() {
    DebugBreak();
}

#endif

#endif
