#pragma once
#ifndef _SCRIPTMACHINE_HPP_
#define _SCRIPTMACHINE_HPP_
#include <string>
#include <map>

#define SCM_CONDITIONAL_MASK 0xF000

struct SCMException
{
	virtual ~SCMException() { }
	virtual std::string what() const = 0;
};

struct IllegalInstruction : SCMException
{
	SCMOpcode opcode;
	unsigned int offset;

	IllegalInstruction(SCMOpcode opcode, unsigned int offset)
		: opcode(opcode), offset(offset) { }

	std::string what() const {
		std::stringstream ss;
		ss << "Illegal Instruction " <<
			  std::setfill('0') << std::setw(4) << std::hex << opcode <<
			  " encountered at offset " <<
			  std::setfill('0') << std::setw(4) << std::hex << offset;
		return ss.str();
	}
};

struct UnknownType : SCMException
{
	SCMByte type;
	unsigned int offset;

	UnknownType(SCMByte type, unsigned int offset)
		: type(type), offset(offset) {}

	std::string what() const {
		std::stringstream ss;
		ss << "Unkown data type " <<
			  std::setfill('0') << std::hex << static_cast<unsigned int>(type) <<
			  " encountered at offset " <<
			  std::setfill('0') << std::hex << offset;
		return ss.str();
	}
};

struct SCMMicrocode {
	std::string name;
	int parameters;
};

typedef std::map<SCMOpcode, SCMMicrocode> SCMMicrocodeTable;

SCMMicrocodeTable knownOps;

enum SCMType {
	EndOfArgList = 0x00,
	TInt32       = 0x01,
	TGlobal      = 0x02,
	TLocal       = 0x03,
	TInt8        = 0x04,
	TInt16       = 0x05,
	TFloat16     = 0x06,
	TString      = 0x09,
};

struct SCMTypeInfo {
	uint8_t size;
};

typedef std::map<SCMType, SCMTypeInfo> SCMTypeInfoTable;

SCMTypeInfoTable typeData = {
	{TInt8,   {1}},
	{TInt16,  {2}},
	{TInt32,  {4}},
	{TGlobal, {2}},
	{TLocal,  {2}},
	{TFloat16,{2}},
	{EndOfArgList, {0}},
};

#endif
