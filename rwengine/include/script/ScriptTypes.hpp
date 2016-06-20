#pragma once
#ifndef _SCRIPTTYPES_HPP_
#define _SCRIPTTYPES_HPP_
#include <rw/defines.hpp>

#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include <functional>

class PickupObject;
class CutsceneObject;
class VehicleObject;
class CharacterObject;
class InstanceObject;
class PlayerController;
class GameObject;
class ScriptMachine;
class ScriptModule;
struct SCMThread;

class GameState;
class GameWorld;

typedef uint16_t SCMOpcode;
typedef char SCMByte;
typedef unsigned int SCMAddress;

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
		int32_t integer;
		float real;
		char string[8];
		void* globalPtr;
		int32_t* globalInteger;
		float* globalReal;
	};

	int integerValue() const 
	{
		switch (type)
		{
		case TGlobal:
		case TLocal:
			return *globalInteger;
		case TInt8:
		case TInt16:
		case TInt32:
			return integer;
		default:
			RW_ERROR("Unhandled type");
			return 0;
		}
	}
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
	// Helper method to get the current state
	GameState* getState() const;
	GameWorld* getWorld() const;

	const SCMOpcodeParameter& operator[](unsigned int arg) const
	{
		return parameters->at(arg);
	}

	template <class T>
	GameObject* getObject(unsigned int arg) const;

    GameObject* getPlayerCharacter(unsigned int player) const;
};

template<> GameObject* ScriptArguments::getObject<InstanceObject>(unsigned int arg) const;
template<> GameObject* ScriptArguments::getObject<CharacterObject>(unsigned int arg) const;
template<> GameObject* ScriptArguments::getObject<VehicleObject>(unsigned int arg) const;
template<> GameObject* ScriptArguments::getObject<CutsceneObject>(unsigned int arg) const;
template<> GameObject* ScriptArguments::getObject<PickupObject>(unsigned int arg) const;
/** Special player-index returning function */
template<> GameObject* ScriptArguments::getObject<PlayerController>(unsigned int arg) const;

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
