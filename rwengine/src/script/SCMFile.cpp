#include <script/SCMFile.hpp>

void SCMFile::loadFile(char *data, unsigned int size) {
    _data = new SCMByte[size];
    std::copy(data, data + size, _data);

    // Bytes required to hop over a jump opcode.
    const unsigned int jumpOpSize = 2u + 1u + 4u;
    const unsigned int jumpParamSize = 2u + 1u;

    _target = static_cast<SCMTarget>(_data[jumpOpSize]);
    globalSectionOffset = jumpOpSize + 1u;
    modelSectionOffset = read<uint32_t>(jumpParamSize) + jumpOpSize + 1u;
    missionSectionOffset =
        read<uint32_t>(modelSectionOffset - jumpOpSize - 1u + jumpParamSize) +
        jumpOpSize + 1u;
    codeSectionOffset =
        read<uint32_t>(missionSectionOffset - jumpOpSize - 1u + jumpParamSize);

    unsigned int model_count = read<uint32_t>(modelSectionOffset);
    models.reserve(model_count);

    int i = modelSectionOffset + sizeof(uint32_t);
    for (unsigned int m = 0; m < model_count; ++m) {
        char model_name[24];
        for (size_t c = 0; c < 24; ++c) {
            model_name[c] = read<char>(i++);
        }
        models.push_back(model_name);
    }

    i = missionSectionOffset;
    mainSize = read<uint32_t>(i);
    i += sizeof(uint32_t);
    missionLargestSize = read<uint32_t>(i);
    i += sizeof(uint32_t);
    unsigned int missionCount = read<uint32_t>(i);
    missionOffsets.reserve(missionCount);
    i += sizeof(uint32_t);

    for (unsigned int m = 0; m < missionCount; ++m) {
        missionOffsets.push_back(read<uint32_t>(i));
        i += sizeof(uint32_t);
    }
}
