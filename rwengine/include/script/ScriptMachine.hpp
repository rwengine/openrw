#pragma once
#ifndef _SCRIPTMACHINE_HPP_
#define _SCRIPTMACHINE_HPP_
#include <script/ScriptTypes.hpp>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>

#define SCM_CONDITIONAL_MASK 0xF000
#define SCM_THREAD_LOCAL_SIZE 256

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

static SCMMicrocodeTable knownOps;

struct SCMThread
{
	typedef unsigned int pc_t;

	std::string name;
	pc_t programCounter;
	/** Number of MS until the thread should be waked (-1 = yeilded) */
	int wakeCounter;
	SCMByte locals[SCM_THREAD_LOCAL_SIZE];
};

class ScriptMachine
{
	SCMFile* _file;
	SCMOpcodes* _ops;

	std::vector<SCMThread> _activeThreads;

	void executeThread(SCMThread& t, int msPassed);

public:
	ScriptMachine(SCMFile* file, SCMOpcodes* ops);
	~ScriptMachine();

	void startThread(SCMThread::pc_t start);

	SCMByte* getGlobals();

	/**
	 * @brief executes threads until they are all in waiting state.
	 */
	void execute(float dt);
};

#endif
