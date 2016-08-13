#include <loaders/LoaderGXT.hpp>
#include <iconv.h>

void LoaderGXT::load(GameTexts &texts, FileHandle &file)
{
	auto data = file->data;

	data += 4; // TKEY

	std::uint32_t blocksize = *(std::uint32_t*)data;

	data += 4;

	auto tdata = data+blocksize+8;

	// This is not supported in GCC 4.8.1
	//std::wstring_convert<std::codecvt<char16_t,char,std::mbstate_t>,char16_t> convert;

	auto icv = iconv_open("UTF-8", "UTF-16");

	for( size_t t = 0; t < blocksize/12; ++t ) {
		size_t offset = *(std::uint32_t*)(data+(t * 12 + 0));
		std::string id(data+(t * 12 + 4));

		// Find the terminating bytes
		size_t bytes = 0;
		for(;; bytes++ ) {
			if(tdata[offset+bytes-1] == 0 && tdata[offset+bytes] == 0) break;
		}
		size_t len = bytes/2;

		size_t outSize = 1024;
		char u8buff[1024];
		char *uwot = u8buff;

		char* strbase = tdata+offset;

#if defined(RW_NETBSD)
		iconv(icv, (const char**)&strbase, &bytes, &uwot, &outSize);
#else
		iconv(icv, &strbase, &bytes, &uwot, &outSize);
#endif

		u8buff[len] = '\0';

		std::string message(u8buff);

		texts.addText(id, message);
	}

	iconv_close(icv);
}
