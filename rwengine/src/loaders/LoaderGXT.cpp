#include <loaders/LoaderGXT.hpp>
#include <data/RWUnicode.hpp>

void LoaderGXT::load(GameTexts &texts, FileHandle &file)
{
	auto data = file->data;

	data += 4; // TKEY

	std::uint32_t blocksize = *(std::uint32_t*)data;

	data += 4;

	auto tdata = data+blocksize+8;

	// This is not supported in GCC 4.8.1
	//std::wstring_convert<std::codecvt<char16_t,char,std::mbstate_t>,char16_t> convert;

	for( size_t t = 0; t < blocksize/12; ++t ) {
		size_t offset = *(std::uint32_t*)(data+(t * 12 + 0));
		std::string id(data+(t * 12 + 4));

		auto message = RW::UnicodeConverter::utf16_to_utf8((char16_t*)(tdata+offset), 512);
		texts.addText(id, message);		
	}
}
