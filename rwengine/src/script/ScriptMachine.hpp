#ifndef RWENGINE_SCRIPTMACHINE_HPP
#define RWENGINE_SCRIPTMACHINE_HPP
#include <array>
#include <iomanip>
#include <list>
#include <rw/defines.hpp>
#include <script/ScriptTypes.hpp>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

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

struct SCMException {
    virtual ~SCMException() {
    }
    virtual std::string what() const = 0;
};

struct IllegalInstruction : SCMException {
    SCMOpcode opcode;
    unsigned int offset;
    std::string thread;

    IllegalInstruction(SCMOpcode opcode, unsigned int offset,
                       const std::string& thread)
        : opcode(opcode), offset(offset), thread(thread) {
    }

    std::string what() const {
        std::stringstream ss;
        ss << "Illegal Instruction " << std::setfill('0') << std::setw(4)
           << std::hex << opcode << " encountered at offset "
           << std::setfill('0') << std::setw(4) << std::hex << offset
           << " on thread " << thread;
        return ss.str();
    }
};

struct UnknownType : SCMException {
    SCMByte type;
    unsigned int offset;
    std::string thread;

    UnknownType(SCMByte type, unsigned int offset, const std::string& thread)
        : type(type), offset(offset), thread(thread) {
    }

    std::string what() const {
        std::stringstream ss;
        ss << "Unkown data type " << std::setfill('0') << std::hex
           << static_cast<unsigned int>(type) << " encountered at offset "
           << std::setfill('0') << std::hex << offset << " on thread "
           << thread;
        return ss.str();
    }
};

static SCMMicrocodeTable knownOps;

struct SCMThread {
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
    std::array<SCMByte, SCM_THREAD_LOCAL_SIZE*(SCM_VARIABLE_SIZE)> locals;
    bool isMission;

    bool finished;

    unsigned int stackDepth;
    /// Stores the return-addresses for calls.
    std::array<pc_t, SCM_STACK_DEPTH> calls;
};

/**
 * Implements the actual fetch-execute mechanism for the game script virtual
 * machine.
 *
 * The unit of functionality is an "instruction", which performs a particular
 * task such as creating a vehicle, retrieving an object's position or declaring
 * a new garage.
 *
 * The VM executes multiple pseudo-threads that execute in series. Each thread
 * is represented by SCMThread, which contains the program counter, stack
 * information
 * the thread name and some thread-local variable space. At startup, a single
 * thread is created at address 0, which begins execution. From there, the
 * script
 * may create additional threads.
 *
 * Within ScriptMachine, each thread's program counter is used to execute an
 * instruction
 * by consuming the correct number of arguments, allowing the next instruction
 * to be found,
 * and then dispatching a call to the opcode's function.
 */
class ScriptMachine {
public:
    ScriptMachine(GameState* state, SCMFile* file, SCMOpcodes* ops);
    ~ScriptMachine();

    SCMFile* getFile() const {
        return _file;
    }

    SCMOpcodes* getOpcodes() const {
        return _ops;
    }

    void startThread(SCMThread::pc_t start, bool mission = false);

    std::list<SCMThread>& getThreads() {
        return _activeThreads;
    }

    SCMByte* getGlobals();
    std::vector<SCMByte>& getGlobalData() {
        return globalData;
    }

    GameState* getState() const {
        return state;
    }

    /**
     * @brief executes threads until they are all in waiting state.
     */
    void execute(float dt);

private:
    SCMFile* _file;
    SCMOpcodes* _ops;
    GameState* state;

    std::list<SCMThread> _activeThreads;

    void executeThread(SCMThread& t, int msPassed);

    std::vector<SCMByte> globalData;
};

#endif
