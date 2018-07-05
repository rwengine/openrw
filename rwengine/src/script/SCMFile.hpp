#ifndef _RWENGINE_SCMFILE_HPP_
#define _RWENGINE_SCMFILE_HPP_

#include "script/ScriptTypes.hpp"

#include <rw/bit_cast.hpp>

#include <cstdint>
#include <string>
#include <vector>

/**
 * @brief Handles in-memory SCM file data including section offsets.
 */
class SCMFile {
public:
    enum SCMTarget { NoTarget = 0, GTAIII = 0xC6, GTAVC = 0x6D, GTASA = 0x73 };

    SCMFile() = default;

    ~SCMFile() {
        delete[] _data;
    }

    void loadFile(char* data, unsigned int size);

    SCMByte* data() const {
        return _data;
    }

    template <class T>
    T read(unsigned int offset) const {
        return bit_cast<T>(*(_data + offset));
    }

    uint32_t getMainSize() const {
        return mainSize;
    }
    uint32_t getLargestMissionSize() const {
        return missionLargestSize;
    }

    const std::vector<std::string>& getModels() const {
        return models;
    }

    const std::vector<unsigned int>& getMissionOffsets() const {
        return missionOffsets;
    }

    uint32_t getGlobalSection() const {
        return globalSectionOffset;
    }
    uint32_t getModelSection() const {
        return modelSectionOffset;
    }
    uint32_t getMissionSection() const {
        return missionSectionOffset;
    }
    uint32_t getCodeSection() const {
        return codeSectionOffset;
    }

    unsigned int getGlobalsSize() const {
        return modelSectionOffset - globalSectionOffset;
    }

private:
    SCMByte* _data = nullptr;

    SCMTarget _target{NoTarget};

    std::vector<std::string> models;

    std::vector<unsigned int> missionOffsets;

    uint32_t mainSize{0};
    uint32_t missionLargestSize{0};

    uint32_t globalSectionOffset{0};
    uint32_t modelSectionOffset{0};
    uint32_t missionSectionOffset{0};
    uint32_t codeSectionOffset{0};
};

#endif
