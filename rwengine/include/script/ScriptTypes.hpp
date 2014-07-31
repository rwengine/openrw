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
		void** handle;
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
	typedef std::function<void (ScriptMachine*, SCMThread*, SCMParams*)> SCMFunc;
	std::map<SCMOpcode, SCMMicrocode> codes;
};


#define OPC_UNIMPLEMENTED_MSG(code, name) [=](ScriptMachine* m, SCMThread* t, SCMParams* p) { std::cout << #code << " " << name << " unimplemented" << std::endl; }

#define VM_OPCODE_DEF(code) void _opcode_##code##_func(ScriptMachine* m, SCMThread* t, SCMParams* p)
#define VM_CONDOPCODE_DEF(code) bool _opcode_##code##_func(ScriptMachine* m, SCMThread* t, SCMParams* p)

#define VM_OPCODE_DEC(code, parameters, name) codes[code] = SCMMicrocode{name, parameters, SCMOpcodes::SCMFunc(_opcode_##code##_func)}
#define VM_CONDOPCODE_DEC(code, parameters, name) codes[code] = SCMMicrocode{name, parameters, \
	[=](ScriptMachine* m, SCMThread* t, SCMParams* p) { t->conditionResult = _opcode_##code##_func(m, t, p); }}

#define VM_OPCODE_DEC_U(code, parameters, name) codes.insert({code, {name, parameters, OPC_UNIMPLEMENTED_MSG(code, name)}})

#endif
