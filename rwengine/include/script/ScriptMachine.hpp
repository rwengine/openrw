#pragma once
#ifndef _SCRIPTMACHINE_HPP_
#define _SCRIPTMACHINE_HPP_
#include <script/ScriptTypes.hpp>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <stack>
#include <list>
#include <set>

#define SCM_NEGATE_CONDITIONAL_MASK 0x8000
#define SCM_CONDITIONAL_MASK_PASSED 0xFF
#define SCM_THREAD_LOCAL_SIZE 256

/* Maxium size value that can be stored in each memory address.
 * Changing this will break saves.
 */
#define SCM_VARIABLE_SIZE 4
#define SCM_STACK_DEPTH 4

class GameState;

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
	typedef SCMAddress pc_t;

	char name[17];
	pc_t baseAddress;
	pc_t programCounter;

	unsigned int conditionCount;
	bool conditionResult;
	std::uint8_t conditionMask;
	bool conditionAND;

	/** Number of MS until the thread should be waked (-1 = yeilded) */
	int wakeCounter;
	std::array<SCMByte, SCM_THREAD_LOCAL_SIZE * (SCM_VARIABLE_SIZE)> locals;
	bool isMission;

	bool finished;

	unsigned int stackDepth;
	/// Stores the return-addresses for calls.
	std::array<pc_t, SCM_STACK_DEPTH> calls;
};

#include <cstring>
/**
 * Stores information about where breakpoints should be triggered.
 * 
 * breakpointFlags stores the state to be checked against.
 */
struct SCMBreakpointInfo
{
	enum /* Breakpoint Flags */ {
		BP_ProgramCounter = 1,
		BP_ThreadName = 2
	};
	uint8_t breakpointFlags;
	SCMThread::pc_t programCounter;
	char threadName[17];

	static SCMBreakpointInfo breakThreadName(char threadName[17])
	{
		SCMBreakpointInfo i;
		i.breakpointFlags = BP_ThreadName;
		std::strncpy(i.threadName, threadName, 17);
		return i;
	}
};

/**
 * Information about breakpoints that have been hit
 */
struct SCMBreakpoint
{
	SCMThread::pc_t pc;
	SCMThread* thread;
	ScriptMachine* vm;
	ScriptFunctionMeta* function;
	ScriptArguments* args;
	/** The breakpoint entry that triggered this breakpoint */
	SCMBreakpointInfo* info;
};

/**
 * Implements the actual fetch-execute mechanism for the game script virtual machine.
 * 
 * The unit of functionality is an "instruction", which performs a particular
 * task such as creating a vehicle, retrieving an object's position or declaring
 * a new garage.
 * 
 * The VM executes multiple pseudo-threads that execute in series. Each thread
 * is represented by SCMThread, which contains the program counter, stack information
 * the thread name and some thread-local variable space. At startup, a single
 * thread is created at address 0, which begins execution. From there, the script
 * may create additional threads.
 *
 * Within ScriptMachine, each thread's program counter is used to execute an instruction
 * by consuming the correct number of arguments, allowing the next instruction to be found,
 * and then dispatching a call to the opcode's function.
 *
 * Breakpoints can be set which will call the breakpoint hander, where it is possible
 * to halt execution by refusing to return until the handler is ready to continue.
 */
class ScriptMachine
{
public:
	ScriptMachine(GameState* state, SCMFile* file, SCMOpcodes* ops);
	~ScriptMachine();

	SCMFile* getFile() const { return _file; }

    SCMOpcodes* getOpcodes() const { return _ops; }

	void startThread(SCMThread::pc_t start, bool mission = false);

	std::list<SCMThread>& getThreads() { return _activeThreads; }

	SCMByte* getGlobals();
	std::vector<SCMByte>& getGlobalData() { return globalData; }

	GameState* getState() const { return state; }
	
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
	 */
	void addBreakpoint(const SCMBreakpointInfo& bpi);

	/**
	 * Removes a breakpoint.
	 */
	void removeBreakpoint(const SCMBreakpointInfo& bpi);

    /**
     * Interupt VM execution at the start of the next instruction
     */
    void interuptNext();

	/**
	 * @brief executes threads until they are all in waiting state.
	 */
	void execute(float dt);
	
private:
	SCMFile* _file;
	SCMOpcodes* _ops;
	GameState* state;
    bool interupt;

	std::list<SCMThread> _activeThreads;

	void executeThread(SCMThread& t, int msPassed);

	SCMBreakpointInfo* findBreakpoint(SCMThread& t, SCMThread::pc_t pc);

	std::vector<SCMByte> globalData;

	BreakpointHandler bpHandler;
	std::vector<SCMBreakpointInfo> breakpoints;
};

#endif
