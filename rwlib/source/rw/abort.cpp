#include <functional>

#if RW_DEBUG
std::function<void()> _rw_abort_cb[2] = {nullptr, nullptr};
#endif
