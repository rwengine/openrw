#pragma once
#ifndef _SCRIPTMACHINE_HPP_
#define _SCRIPTMACHINE_HPP_
#include <script/ScriptTypes.hpp>
#include <sstream>
#include <iomanip>
#include <string>
#include <stack>
#include <set>

#define SCM_NEGATE_CONDITIONAL_MASK 0x8000
#define SCM_CONDITIONAL_MASK_PASSED 0xFF
#define SCM_THREAD_LOCAL_SIZE 256

/* as shipped, SCM variables are 4 bytes wide, this isn't enough for 64-bit
 * pointers, so we re-allocate the global and local space taking into account
 * the native pointer size */
#define SCM_VARIABLE_SIZE sizeof(void*)

class GameWorld;

class SCMFile;

struct SCMException
{
	virtual ~SCMException() { }
	virtual std::string what() const = 0;
};

struct IllegalInstruction : SCMException
{
	SCMOpcode opcode;
	unsigned int offset;
	std::string thread;

	IllegalInstruction(SCMOpcode opcode, unsigned int offset, const std::string& thread)
		: opcode(opcode), offset(offset), thread(thread) { }

	std::string what() const {
		std::stringstream ss;
		ss << "Illegal Instruction " <<
			  std::setfill('0') << std::setw(4) << std::hex << opcode <<
			  " encountered at offset " <<
			  std::setfill('0') << std::setw(4) << std::hex << offset <<
			  " on thread " << thread;
		return ss.str();
	}
};

struct UnknownType : SCMException
{
	SCMByte type;
	unsigned int offset;
	std::string thread;

	UnknownType(SCMByte type, unsigned int offset, const std::string& thread)
		: type(type), offset(offset), thread(thread) {}

	std::string what() const {
		std::stringstream ss;
		ss << "Unkown data type " <<
			  std::setfill('0') << std::hex << static_cast<unsigned int>(type) <<
			  " encountered at offset " <<
			  std::setfill('0') << std::hex << offset <<
			  " on thread " << thread;
		return ss.str();
	}
};

struct UnimplementedOpcode : SCMException
{
	SCMOpcode opcode;
	SCMParams parameters;

	UnimplementedOpcode(SCMOpcode opcode, SCMParams parameters)
		: opcode(opcode), parameters(parameters) {}

	std::string what() const {
		std::stringstream ss;
		ss << "Unimplemented opcode " <<
			  std::setfill('0') << std::hex << opcode <<
			  " called with parameters:\n";
		int i = 0;
		for(const SCMOpcodeParameter& p : parameters) {
			ss << (i++) << " " << p.type << " ";
			switch (p.type) {
			case TInt8:
			case TInt16:
			case TInt32:
				ss << p.integer;
				break;
			case TFloat16:
				ss << p.real;
				break;
			case TGlobal:
				ss << "Global: " << p.globalPtr;
				break;
			default:
				ss << "Unprintable";
				break;
			}
			ss << "\n";
		}
		return ss.str();
	}
};

static SCMMicrocodeTable knownOps;

struct SCMThread
{
	typedef unsigned int pc_t;

	std::string name;
	pc_t baseAddress;
	pc_t programCounter;

	unsigned int conditionCount;
	bool conditionResult;
	std::uint8_t conditionMask;
	bool conditionAND;

	/** Number of MS until the thread should be waked (-1 = yeilded) */
	int wakeCounter;
	SCMByte locals[SCM_THREAD_LOCAL_SIZE * (SCM_VARIABLE_SIZE)];
	bool isMission;

	bool finished;

	/// Stores the return-addresses for calls.
	std::stack<pc_t> calls;
};

/**
 * Breakpoint callback information
 */
struct SCMBreakpoint
{
	SCMThread::pc_t pc;
	SCMThread* thread;
	ScriptMachine* vm;
	ScriptFunctionMeta* function;
	ScriptArguments* args;
};

class ScriptMachine
{
public:
	ScriptMachine(GameWorld* world, SCMFile* file, SCMOpcodes* ops);
	~ScriptMachine();

	SCMFile* getFile() const { return _file; }

	void startThread(SCMThread::pc_t start, bool mission = false);

	SCMByte* getGlobals();

	GameWorld* getWorld() const { return _world; }
	
	typedef std::function<void (const SCMBreakpoint&)> BreakpointHandler;

	/**
	 * Set the breakpoint handler callback.
	 *
	 * When the VM reaches an instruction marked as a brekapoint
	 * by addBreakpoint, the handler will be called with information
	 * about the state of the VM and the active thread.
	 */
	void setBreakpointHandler(const BreakpointHandler& handler);

	/**
	 * Adds a breakpoint
	 * @param pc The instruction address to break on.
	 */
	void addBreakpoint(SCMThread::pc_t pc);

	/**
	 * Removes a breakpoint.
	 */
	void removeBreakpoint(SCMThread::pc_t pc);

	/**
	 * @brief executes threads until they are all in waiting state.
	 */
	void execute(float dt);
	
private:
	SCMFile* _file;
	SCMOpcodes* _ops;
	GameWorld* _world;

	std::vector<SCMThread> _activeThreads;

	void executeThread(SCMThread& t, int msPassed);

	SCMByte* _globals;

	BreakpointHandler bpHandler;
	std::set<SCMThread::pc_t> breakpoints;
};

#endif
