#ifndef RWENGINE_SCRIPTTYPES_HPP
#define RWENGINE_SCRIPTTYPES_HPP
#include <rw/defines.hpp>
#include <objects/ObjectTypes.hpp>

#include <glm/glm.hpp>

#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include <functional>

// Include the concrete types
#include <objects/PickupObject.hpp>
#include <objects/CutsceneObject.hpp>
#include <objects/VehicleObject.hpp>
#include <objects/InstanceObject.hpp>
#include <objects/CharacterObject.hpp>
class PlayerController;
class ScriptMachine;
class ScriptModule;
struct SCMThread;

class GameState;
class GameWorld;

typedef uint16_t SCMOpcode;
typedef char SCMByte;
typedef unsigned int SCMAddress;

///////////////////////////////////////////////////////////
// Script Implementation Types

using ScriptInt = int32_t;
using ScriptLabel = int32_t;
using ScriptIntGlobal = int32_t&;
using ScriptIntLocal = int32_t&;
using ScriptFloat = float;
using ScriptFloatGlobal = float&;
using ScriptFloatLocal = float&;
using ScriptBoolean = int32_t;

using ScriptVec2 = glm::vec2;
using ScriptVec3 = glm::vec3;

using ScriptRGB = glm::u8vec3;
using ScriptRGBA = glm::u8vec4;

using ScriptModelID = int32_t;
using ScriptString = char[8];


/**
 * Helper class for transparently handling integer handles for objects
 */
template <class T>
struct ScriptObjectType {
	/**
	 * @brief m_id VM Memory containing the object handler
	 */
	ScriptInt* m_id;
	/**
	 * @brief m_object Real object instance
	 */
	T* m_object;

	ScriptObjectType(ScriptInt* var, GameObject* object)
		: m_id(var)
		, m_object(static_cast<T*>(object))
	{
	}

	ScriptObjectType(ScriptInt* var, T* object)
		: m_id(var)
		, m_object(object)
	{
	}

	/**
	 * Assigns the memory location to the ID of the given instance
	 */
	T* operator = (T* object)
	{
		RW_CHECK(m_id != nullptr, "ScriptObjectType has pointer to null memory location");
		*m_id = object->getScriptID();
		return object;
	}

	T* operator -> () const
	{
		RW_CHECK(m_object != nullptr, "Dereferencing ScriptObjectType with null instance");
		return m_object;
	}
};

using ScriptObject = ScriptObjectType<InstanceObject>;
using ScriptPlayer = ScriptObjectType<PlayerController>;
using ScriptVehicle = ScriptObjectType<VehicleObject>;
using ScriptCharacter = ScriptObjectType<CharacterObject>;
using ScriptPickup = ScriptObjectType<PickupObject>;

struct VehicleGenerator;
struct BlipData;
struct GarageInfo;

using ScriptVehicleGenerator = ScriptObjectType<VehicleGenerator>;
using ScriptBlip = ScriptObjectType<BlipData>;
using ScriptGarage = ScriptObjectType<GarageInfo>;

/// @todo replace these with real types for sounds etc.
using ScriptSound = ScriptObjectType<int>;
using ScriptPhone = ScriptObjectType<int>;
using ScriptFire = ScriptObjectType<int>;
using ScriptSphere = ScriptObjectType<int>;

/// @todo replace these with real enums
using ScriptModelID = int;
using ScriptPedType = int;
using ScriptDrivingMode = int;
using ScriptMission = int;
using ScriptPad = int;
using ScriptButton = int;
using ScriptModel = int;
using ScriptWeaponType = int;
using ScriptThreat = int;
using ScriptCarLock = int;
using ScriptCarColour = int;
using ScriptCamMode = int;
using ScriptChangeCamMode = int;
using ScriptBlipColour = int;
using ScriptBlipDisplay = int;
using ScriptFade = int;
using ScriptShadow = int;
using ScriptContact = int;
using ScriptWeather = int;
using ScriptFollowRoute = int;
using ScriptExplosion = int;
using ScriptCarBomb = int;
using ScriptGang = int;
using ScriptPedStat = int;
using ScriptAnim = int;
using ScriptCoronaType = int;
using ScriptFlareType = int;
using ScriptPObject = int;
using ScriptRadarSprite = int;
using ScriptPedGrp = int;
using ScriptCamZoom = int;
using ScriptFont = int;
using ScriptWaitState = int;
using ScriptMotionBlur = int;
using ScriptStatus = int;
using ScriptTimer = int;
using ScriptCounterDisplay = int;
using ScriptLevel = int;
using ScriptHudFlash = int;
using ScriptDoor = int;
using ScriptRadio = int;
using ScriptParticle = int;
using ScriptTempact = int;
using ScriptSoundType = int;
using ScriptPickupType = int;
using ScriptGarageType = int;

///////////////////////////////////////////////////////////
// Script Bytecode Types

/**
 * @brief Enum of opcode arg types
 */
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

	float realValue() const
	{
		switch (type)
		{
		case TGlobal:
		case TLocal:
			return *globalReal;
		case TFloat16:
			return real;
		default:
			RW_ERROR("Unhandled type");
			return 0;
		}
	}

	bool isLvalue() const {
		return type == TLocal || type == TGlobal;
	}

	int32_t* handleValue() const {
		return globalInteger;
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

	int getModel(unsigned int arg) const;

	template <class T>
	GameObject* getObject(unsigned int arg) const;

	GameObject* getPlayerCharacter(unsigned int player) const;

	template <class T>
	T getParameter(unsigned int arg) const;

	template <class T>
	T& getParameterRef(unsigned int arg) const;

	/**
	 * Returns a handle for the object of type T at the argument index.
	 */
	template <class T>
	ScriptObjectType<T> getScriptObject(unsigned int arg) const;
};

template<> GameObject* ScriptArguments::getObject<InstanceObject>(unsigned int arg) const;
template<> GameObject* ScriptArguments::getObject<CharacterObject>(unsigned int arg) const;
template<> GameObject* ScriptArguments::getObject<VehicleObject>(unsigned int arg) const;
template<> GameObject* ScriptArguments::getObject<CutsceneObject>(unsigned int arg) const;
template<> GameObject* ScriptArguments::getObject<PickupObject>(unsigned int arg) const;
/** Special player-index returning function */
template<> GameObject* ScriptArguments::getObject<PlayerController>(unsigned int arg) const;

template<> ScriptFloat ScriptArguments::getParameter<ScriptFloat>(unsigned int arg) const;
template<> ScriptInt ScriptArguments::getParameter<ScriptInt>(unsigned int arg) const;
template<> char const* ScriptArguments::getParameter<char const*>(unsigned int arg) const;
template<> ScriptVec2 ScriptArguments::getParameter<ScriptVec2>(unsigned int arg) const;
template<> ScriptVec3 ScriptArguments::getParameter<ScriptVec3>(unsigned int arg) const;
template<> ScriptRGB ScriptArguments::getParameter<ScriptRGB>(unsigned int arg) const;
template<> ScriptRGBA ScriptArguments::getParameter<ScriptRGBA>(unsigned int arg) const;

template<> ScriptFloat& ScriptArguments::getParameterRef<ScriptFloat>(unsigned int arg) const;
template<> ScriptInt& ScriptArguments::getParameterRef<ScriptInt>(unsigned int arg) const;

///////////////////////////////////////////////////////////
// Script Object specialisations
template <> ScriptObjectType<VehicleObject> ScriptArguments::getScriptObject(unsigned int arg) const;
template <> ScriptObjectType<InstanceObject> ScriptArguments::getScriptObject(unsigned int arg) const;
template <> ScriptObjectType<CharacterObject> ScriptArguments::getScriptObject(unsigned int arg) const;
template <> ScriptObjectType<PlayerController> ScriptArguments::getScriptObject(unsigned int arg) const;
template <> ScriptObjectType<CutsceneObject> ScriptArguments::getScriptObject(unsigned int arg) const;
template <> ScriptObjectType<PickupObject> ScriptArguments::getScriptObject(unsigned int arg) const;
template <> ScriptObjectType<BlipData> ScriptArguments::getScriptObject(unsigned int arg) const;
template <> ScriptObjectType<VehicleGenerator> ScriptArguments::getScriptObject(unsigned int arg) const;
template <> ScriptObjectType<GarageInfo> ScriptArguments::getScriptObject(unsigned int arg) const;

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
