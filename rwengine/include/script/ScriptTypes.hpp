#pragma once
#ifndef _SCRIPTTYPES_HPP_
#define _SCRIPTTYPES_HPP_
#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include <functional>

class ScriptMachine;
class ScriptModule;
struct SCMThread;

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
	
	template<class T> T* handleOf() const { return static_cast<T*>(*handle); }
};

typedef std::vector<SCMOpcodeParameter> SCMParams;

class ScriptArguments
{
	const SCMParams* parameters;
	SCMThread* thread;
	ScriptMachine* machine;
	
public:
	ScriptArguments(const SCMParams* p, SCMThread* t, ScriptMachine* m)
		: parameters(p), thread(t), machine(m) { }
	
	const SCMParams& getParameters() const { return *parameters; }
	SCMThread* getThread() const { return thread; }
	ScriptMachine* getVM() const { return machine; }
	
	const SCMOpcodeParameter& operator[](unsigned int arg) const
	{
		return parameters->at(arg);
	}
};

typedef std::function<void (const ScriptArguments&)> ScriptFunction;
typedef std::function<bool (const ScriptArguments&)> ScriptFunctionBoolean;
typedef uint16_t ScriptFunctionID;

struct ScriptFunctionMeta
{
	ScriptFunction function;
	int arguments;
	bool conditional;
	/** API name for this function */
	const std::string signature;
	/** Human friendly description */
	const std::string description;
};

struct SCMMicrocode {
	std::string name;
	int parameters;
	std::function<void (ScriptMachine*, SCMThread*, std::vector<SCMOpcodeParameter>*)> func;
};

typedef std::map<SCMOpcode, SCMMicrocode> SCMMicrocodeTable;

struct SCMOpcodes
{
	std::vector<ScriptModule*> modules;
	
	~SCMOpcodes();
	
	bool findOpcode(ScriptFunctionID id, ScriptFunctionMeta** out);
};


#endif
