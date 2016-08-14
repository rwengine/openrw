#pragma once
#include <uchar.h>

namespace RW
{
	class UnicodeConverter
	{
	public:
		static std::string utf16_to_utf8(char16_t* source, size_t maxlength)
		{
			size_t len = 0, mbseq = 0;
			std::string result(maxlength, '\0');
			mbstate_t mbs{};
			char16_t* u16offset = source;
			while(*u16offset)
			{
				mbseq = c16rtomb(&result[len], *u16offset, &mbs);				
				if( mbseq > maxlength ) break;
				++u16offset;
				len += mbseq;
			}
			result.resize(len);
			return result;
		}
	};
}
