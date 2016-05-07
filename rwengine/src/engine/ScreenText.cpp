#include <engine/ScreenText.hpp>

void ScreenText::tick(float dt)
{
	int millis = dt * 1000;

	// Remove all the immedate text
	m_textQueues[static_cast<size_t>(ScreenTextType::Immediate)].clear();

	for (unsigned int t = 0; t < m_textQueues.size(); ++t)
	{
		for (unsigned int i = 0; i < m_textQueues[t].size();)
		{
			auto& big = m_textQueues[t][i];

			big.displayedMS += millis;
			if (big.displayedMS >= big.durationMS)
			{
				m_textQueues[t].erase(m_textQueues[t].begin()+i);
			}
			else
			{
				++i;
			}
		}
	}
}

ScreenTextEntry ScreenTextEntry::makeBig(const std::string& id, const std::string& str, int style, int durationMS)
{
	switch(style) {
	case 1:
		return {
			str,
			{320.f, 400.f},
			1,
			50,
			{ 3,  3,   0,   0},
			{20, 20, 200},
			1,
			durationMS,
			0,
			600,
			id
		};
	case 2:
		return {
			str,
			{620.f, 380.f},
			1,
			30,
			{  3,   3, 0,   0},
			{205, 162, 7},
			2,
			durationMS,
			0,
			600,
			id
		};
	default:
		RW_ERROR("Unhandled text style");
		break;
	}

	return {
		"Error",
		{320.f, 400.f},
		1,
		50,
		{20, 20,   0,   0},
		{20, 20, 200},
		1,
		durationMS,
		0,
		600,
		id
	};
}

ScreenTextEntry ScreenTextEntry::makeHighPriority(const std::string& id, const std::string& str, int durationMS)
{
	return {
		str,
		{320.f, 420.f},
		2,
		18,
		{0, 0,   0,   0},
		{255, 255, 255},
		1,
		durationMS,
		0,
		50,
		id
	};
}

ScreenTextEntry ScreenTextEntry::makeHelp(const std::string& id, const std::string& str)
{
	return {
		str,
		{20.f, 20.f},
		2,
		18,
		{  0,   0,   0, 255},
		{255, 255, 255},
		0,
		5000,
		0,
		35,
		id
	};
}
