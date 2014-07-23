#pragma once
#ifndef _SCMFILE_HPP_
#define _SCMFILE_HPP_
#include <vector>
#include <string>
#include <script/ScriptTypes.hpp>

/**
 * @brief Handles in-memory SCM file data including section offsets.
 */
class SCMFile {
public:

	typedef std::uint32_t uint32;
	typedef std::uint16_t u16;
	typedef std::uint8_t  u8;

	enum SCMTarget {
		NoTarget = 0,
		GTAIII = 0xC6,
		GTAVC  = 0x6D,
		GTASA  = 0x73
	};

	SCMFile()
		: _data(nullptr), _target(NoTarget), mainOffset(0),
		  mainSize(0), missionLargestSize(0)
	{}

	~SCMFile()
	{
		delete[] _data;
	}

	void loadFile(char* data, unsigned int size);

	SCMByte* data() const { return _data; }

	template<class T> T read(unsigned int offset) const
	{
		return *(T*)(_data+offset);
	}

	uint32 getMainSize() const { return mainSize; }
	uint32 getLargestMissionSize() const { return missionLargestSize; }

	const std::vector<std::string>& getModels() const { return models; }

	const std::vector<unsigned int>& getMissionOffsets() const { return missionOffsets; }

	std::uint32_t getGlobalSection() const { return globalSectionOffset; }
	std::uint32_t getModelSection() const { return modelSectionOffset; }
	std::uint32_t getMissionSection() const { return missionSectionOffset; }
	std::uint32_t getCodeSection() const { return codeSectionOffset; }

private:

	SCMByte* _data;

	SCMTarget _target;

	std::vector<std::string> models;

	uint32 mainOffset;
	std::vector<unsigned int> missionOffsets;

	uint32 mainSize;
	uint32 missionLargestSize;

	uint32 globalSectionOffset;
	uint32 modelSectionOffset;
	uint32 missionSectionOffset;
	uint32 codeSectionOffset;
};

#endif
