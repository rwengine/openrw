#include <BinaryStream.hpp>

#include <cstring>
#include <iostream>
#include <fstream>

namespace RW
{

std::unique_ptr<BinaryStream> BinaryStream::parse(std::string filename)
{
	std::ifstream dfile(filename);
	if ( ! dfile.is_open()) {
		std::cerr << "Error opening file " << filename << std::endl;
		return nullptr;
	}

	dfile.seekg(0, std::ios_base::end);
	size_t length = dfile.tellg();
	dfile.seekg(0);
	char *data = new char[length];
	dfile.read(data, length);
	// std::cout << "File is " << length << " bytes" << std::endl << std::endl;

	auto BS = std::unique_ptr<BinaryStream>(new BinaryStream);

	// Set file's ACTUAL length
	auto header = reinterpret_cast<nativeSectionHeader_t *>(data);
	length = header->size + sizeof(nativeSectionHeader_t);

	sectionHeader_t *prevHeader = nullptr;

	size_t offset = 0;
	while (offset < length) {
		nativeSectionHeader_t *sectionHeader = reinterpret_cast<nativeSectionHeader_t *>(data + offset);
		sectionHeader_t *sec = new sectionHeader_t;
		sec->ID = sectionHeader->ID;
		sec->size = sectionHeader->size;
		sec->version = sectionHeader->version;
		if (prevHeader == nullptr)
			BS->rootHeader = sec;
		else
			prevHeader->next = sec;

		if (sectionHeader->ID == 0) {
			std::cout << "Section ID is ZERO! Abort!" << std::endl;
			break;
		}

		std::cout << "Section " << std::hex << sectionHeader->ID
			<< " (" << sectionIdString(sectionHeader->ID) << ")"
			<< " - " << std::dec << sectionHeader->size << " bytes" << std::endl;
/*
		std::cout << "Offset " << std::hex << offset << std::endl;
*/

		size_t bytesOfData = 0;
		switch (sectionHeader->ID) {
		case STRUCT:
			bytesOfData = sectionHeader->size;
			sec->data = new uint8_t[bytesOfData];
			memcpy(sec->data, data + offset + sizeof(nativeSectionHeader_t), bytesOfData);
			break;
		}
		// std::cout << "It has " << std::dec << bytesOfData << " bytes of data!" << std::endl;
		offset += sizeof(nativeSectionHeader_t) + bytesOfData;

		// std::cout << std::endl;

		prevHeader = sec;
	}

	delete[] data;

	return BS;
}

std::string BinaryStream::sectionIdString(uint32_t id)
{
	switch (id) {
	case STRUCT: return "STRUCT";
	case EXTENSION: return "EXTENSION";
	case TEXTURE_NATIVE: return "TEXTURE_NATIVE";
	case TEXTURE_DICTIONARY: return "TEXTURE_DICTIONARY";
	default: return "UNKNOWN";
	}
}

}
