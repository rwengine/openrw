#pragma once
#ifndef _SCRIPTTYPES_HPP_
#define _SCRIPTTYPES_HPP_
#include <cstdint>
#include <map>
#include <vector>
#include <functional>

class ScriptMachine;
class SCMThread;

typedef uint16_t SCMOpcode;
typedef char SCMByte;

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

static SCMTypeInfoTable typeData = {
	{TInt8,   {1}},
	{TInt16,  {2}},
	{TInt32,  {4}},
	{TGlobal, {2}},
	{TLocal,  {2}},
	{TFloat16,{2}},
	{EndOfArgList, {0}},
};


struct SCMOpcodeParameter {
	SCMType type;
	union {
		int integer;
		float real;
		char string[8];
		void* globalPtr;
		int* globalInteger;
		float* globalReal;
	};
};

typedef std::vector<SCMOpcodeParameter> SCMParams;

struct SCMMicrocode {
	std::string name;
	int parameters;
	std::function<void (ScriptMachine*, SCMThread*, std::vector<SCMOpcodeParameter>*)> func;
};

typedef std::map<SCMOpcode, SCMMicrocode> SCMMicrocodeTable;

struct SCMOpcodes
{
	std::map<SCMOpcode, SCMMicrocode> codes;
};

#endif
