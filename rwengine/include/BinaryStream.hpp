#pragma once
#ifndef _BINARYSTREAM_HPP_
#define _BINARYSTREAM_HPP_

#include <string>
#include <memory>

namespace RW
{

class BinaryStream
{
private:
	struct nativeSectionHeader_t
	{
		uint32_t ID;
		uint32_t size;
		uint32_t version;
	};
public:
	enum {
		STRUCT = 0x0001,
		EXTENSION = 0x0003,
		TEXTURE_NATIVE = 0x0015,
		TEXTURE_DICTIONARY = 0x0016,
	};

	struct sectionHeader_t {
		uint32_t ID;
		uint32_t size;
		uint32_t version;

		uint8_t *data = nullptr;
		sectionHeader_t *next = nullptr;
	} *rootHeader;

	static std::unique_ptr<BinaryStream> parse(const std::string& filename);

	static std::string sectionIdString(uint32_t id);
};

}

#endif