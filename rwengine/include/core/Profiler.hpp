#ifndef _RWENGINE_PROFILER_HPP_
#define _RWENGINE_PROFILER_HPP_

#if RW_PROFILER
#include <rw/defines.hpp>
#include <vector>
#include <string>
#include <chrono>
#include <stack>
#define time_unit std::chrono::microseconds

namespace perf
{

struct ProfileEntry
{
	std::string label;
	int64_t start;
	int64_t end;
	std::vector<ProfileEntry> childProfiles;
};

class Profiler
{
	ProfileEntry frame;
	std::chrono::high_resolution_clock::time_point frameBegin;
	std::stack<ProfileEntry> currentStack;

public:

	static Profiler& get()
	{
		static Profiler profile;
		return profile;
	}

	const ProfileEntry& getFrame() const
	{
		return frame;
	}

	void startFrame()
	{
		frameBegin = std::chrono::high_resolution_clock::now();
		frame = { "Frame", 0, 0, {} };
	}

	void beginEvent(const std::string& label)
	{
		auto now = std::chrono::duration_cast<time_unit>(
					std::chrono::high_resolution_clock::now() - frameBegin);
		currentStack.push({label, now.count(), 0, {} });
	}

	void endEvent()
	{
		auto now = std::chrono::duration_cast<time_unit>(
					std::chrono::high_resolution_clock::now() - frameBegin);
		RW_CHECK(currentStack.size() > 0, "Perf stack is empty");
		currentStack.top().end = now.count();
		if (currentStack.size() == 1) {
			frame.childProfiles.push_back(currentStack.top());
			currentStack.pop();
		}
		else {
			auto tmp = currentStack.top();
			currentStack.pop();
			currentStack.top().childProfiles.push_back(tmp);
		}
	}
};
}
#define RW_PROFILE_FRAME_BOUNDARY() \
	perf::Profiler::get().startFrame();
#define RW_PROFILE_BEGIN(label) \
	perf::Profiler::get().beginEvent(label);
#define RW_PROFILE_END() \
	perf::Profiler::get().endEvent();
#else
#define RW_PROFILE_BEGIN(label)
#define RW_PROFILE_END()
#endif

#endif
