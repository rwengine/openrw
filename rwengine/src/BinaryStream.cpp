#include "BinaryStream.hpp"

#include <cstring>
#include <fstream>
#include <rw/debug.hpp>

namespace RW {

std::unique_ptr<BinaryStream> BinaryStream::parse(const std::string &filename) {
    std::ifstream dfile(filename, std::ios_base::binary);
    if (!dfile.is_open()) {
        RW_ERROR("Error opening file " << filename);
        return nullptr;
    }

    dfile.seekg(0, std::ios_base::end);
    size_t length = dfile.tellg();
    dfile.seekg(0);
    char *data = new char[length];
    dfile.read(data, length);
    // RW_MESSAGE("File is " << length << " bytes");

    auto BS = std::make_unique<BinaryStream>();

    // Set file's ACTUAL length
    auto header = reinterpret_cast<nativeSectionHeader_t *>(data);
    length = header->size + sizeof(nativeSectionHeader_t);

    sectionHeader_t *prevHeader = nullptr;

    size_t offset = 0;
    while (offset < length) {
        nativeSectionHeader_t *sectionHeader =
            reinterpret_cast<nativeSectionHeader_t *>(data + offset);
        sectionHeader_t *sec = new sectionHeader_t;
        sec->ID = sectionHeader->ID;
        sec->size = sectionHeader->size;
        sec->version = sectionHeader->version;
        if (prevHeader == nullptr)
            BS->rootHeader = sec;
        else
            prevHeader->next = sec;

        if (sectionHeader->ID == 0) {
            RW_ERROR("Section ID is ZERO! Abort!");
            break;
        }

        RW_MESSAGE("Section " << std::hex << sectionHeader->ID << " ("
                  << sectionIdString(sectionHeader->ID) << ")"
                  << " - " << std::dec << sectionHeader->size << " bytes");
        /*
                RW_MESSAGE("Offset " << std::hex << offset);
        */

        size_t bytesOfData = 0;
        switch (sectionHeader->ID) {
            case STRUCT:
                bytesOfData = sectionHeader->size;
                sec->data = new uint8_t[bytesOfData];
                memcpy(sec->data, data + offset + sizeof(nativeSectionHeader_t),
                       bytesOfData);
                break;
        }
        // RW_MESSAGE("It has " << std::dec << bytesOfData
        //            << " bytes of data!");
        offset += sizeof(nativeSectionHeader_t) + bytesOfData;

        prevHeader = sec;
    }

    delete[] data;

    return BS;
}

std::string BinaryStream::sectionIdString(uint32_t id) {
    switch (id) {
        case STRUCT:
            return "STRUCT";
        case EXTENSION:
            return "EXTENSION";
        case TEXTURE_NATIVE:
            return "TEXTURE_NATIVE";
        case TEXTURE_DICTIONARY:
            return "TEXTURE_DICTIONARY";
        default:
            return "UNKNOWN";
    }
}
}
