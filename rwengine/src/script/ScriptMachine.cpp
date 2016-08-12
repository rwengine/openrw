#include <script/ScriptMachine.hpp>
#include <script/SCMFile.hpp>
#include <script/ScriptModule.hpp>
#include <engine/GameState.hpp>
#include <engine/GameWorld.hpp>
#include <core/Logger.hpp>
#include <cstring>

SCMOpcodes::~SCMOpcodes()
{
	for(auto m : modules)
	{
		delete m;
	}
}

bool SCMOpcodes::findOpcode(ScriptFunctionID id, ScriptFunctionMeta** out)
{
	for(ScriptModule* module : modules)
	{
		if( module->findOpcode(id, out) )
		{
			return true;
		}
	}
	
	return false;
}

void ScriptMachine::executeThread(SCMThread &t, int msPassed)
{
	if( t.wakeCounter > 0 ) {
		t.wakeCounter = std::max( t.wakeCounter - msPassed, 0 );
	}
	if( t.wakeCounter > 0 ) return;
	
    while( t.wakeCounter == 0 ) {
        auto pc = t.programCounter;
        auto opcode = _file->read<SCMOpcode>(pc);

		bool isNegatedConditional = ((opcode & SCM_NEGATE_CONDITIONAL_MASK) == SCM_NEGATE_CONDITIONAL_MASK);
		opcode = opcode & ~SCM_NEGATE_CONDITIONAL_MASK;

		ScriptFunctionMeta* foundcode;
		if( ! _ops->findOpcode(opcode, &foundcode) )
		{
            throw IllegalInstruction(opcode, pc, t.name);
		}
		ScriptFunctionMeta& code = *foundcode;

        pc += sizeof(SCMOpcode);

		SCMParams parameters;
		
		bool hasExtraParameters = code.arguments < 0;
		auto requiredParams = std::abs(code.arguments);

		for( int p = 0; p < requiredParams || hasExtraParameters; ++p ) {
            auto type_r = _file->read<SCMByte>(pc);
			auto type = static_cast<SCMType>(type_r);

			if( type_r > 42 ) {
				// for implicit strings, we need the byte we just read.
				type = TString;
			}
			else {
                pc += sizeof(SCMByte);
			}

			parameters.push_back(SCMOpcodeParameter { type, { 0 } });
			switch(type) {
			case EndOfArgList:
				hasExtraParameters = false;
				break;
			case TInt8:
				parameters.back().integer = _file->read<std::int8_t>(pc);
				pc += sizeof(SCMByte);
				break;
			case TInt16:
				parameters.back().integer = _file->read<std::int16_t>(pc);
				pc += sizeof(SCMByte) * 2;
				break;
			case TGlobal: {
				auto v = _file->read<std::uint16_t>(pc);
				parameters.back().globalPtr = globalData.data() + v; //* SCM_VARIABLE_SIZE;
				if( v >= _file->getGlobalsSize() )
				{
					state->world->logger->error("SCM", "Global Out of bounds! "+ std::to_string(v) + " " + std::to_string(_file->getGlobalsSize()));
				}
				pc += sizeof(SCMByte) * 2;
			}
				break;
			case TLocal: {
				auto v = _file->read<std::uint16_t>(pc);
				parameters.back().globalPtr = t.locals.data() + v * SCM_VARIABLE_SIZE;
				if( v >= SCM_THREAD_LOCAL_SIZE )
				{
					state->world->logger->error("SCM", "Local Out of bounds!");
				}
				pc += sizeof(SCMByte) * 2;
			}
				break;
			case TInt32:
				parameters.back().integer = _file->read<std::int32_t>(pc);
				pc += sizeof(SCMByte) * 4;
				break;
			case TString:
                std::copy(_file->data()+pc, _file->data()+pc+8,
						  parameters.back().string);
                pc += sizeof(SCMByte) * 8;
				break;
			case TFloat16:
                parameters.back().real = _file->read<std::int16_t>(pc) / 16.f;
                pc += sizeof(SCMByte) * 2;
				break;
			default:
                throw UnknownType(type, pc, t.name);
				break;
			};
		}

        ScriptArguments sca(&parameters, &t, this);

#if RW_SCRIPT_DEBUG
		static auto sDebugThreadName = getenv("OPENRW_DEBUG_THREAD");
		if (!sDebugThreadName || strncmp(t.name, sDebugThreadName, 8) == 0)
		{
			printf("%8s %01x %06x %04x %s", t.name, t.conditionResult, t.programCounter, opcode, code.signature.c_str());
			for (auto& a : sca.getParameters())
			{
				if (a.type == SCMType::TString) {
					printf(" %1x:'%s'", a.type, a.string);
				}
				else if (a.type == SCMType::TFloat16) {
					printf(" %1x:%f", a.type, a.realValue());
				}
				else {
					printf(" %1x:%d", a.type, a.integerValue());
				}
			}
			printf("\n");
		}
#endif

        // After debugging has been completed, update the program counter
        t.programCounter = pc;

		if(code.function)
        {
			code.function(sca);
		}

		if(isNegatedConditional) {
			t.conditionResult = !t.conditionResult;
		}

		// Handle conditional results for IF statements.
		if( t.conditionCount > 0 && opcode != 0x00D6 ) /// @todo add conditional flag to opcodes instead of checking for 0x00D6
		{
			--t.conditionCount;
			if ( t.conditionAND )
			{
				if ( t.conditionResult == false ) 
				{
					t.conditionMask = 0;
				}
				else
				{
					// t.conditionMask is already set to 0xFF by the if and opcode.
				}
			}
			else
			{
				t.conditionMask = t.conditionMask || t.conditionResult;
			}
			
			t.conditionResult = (t.conditionMask != 0);
		}
    }
	
	SCMOpcodeParameter p;
	p.globalPtr = (t.locals.data() + 16 * sizeof ( SCMByte ) * 4);
	*p.globalInteger += msPassed;
	p.globalPtr = (t.locals.data() + 17 * sizeof ( SCMByte ) * 4);
	*p.globalInteger += msPassed;
	
	if( t.wakeCounter == -1 ) {
		t.wakeCounter = 0;
	}
}

ScriptMachine::ScriptMachine(GameState* _state, SCMFile *file, SCMOpcodes *ops)
    : _file(file), _ops(ops), state(_state)
{
  // Copy globals
	auto size = _file->getGlobalsSize();
	globalData.resize(size);
  auto offset = _file->getGlobalSection();
	std::copy(_file->data()+offset, _file->data()+offset+size,
	          globalData.begin());
}

ScriptMachine::~ScriptMachine()
{
	delete _file;
	delete _ops;
}

void ScriptMachine::startThread(SCMThread::pc_t start, bool mission)
{
	SCMThread t;
	for(int i = 0; i < SCM_THREAD_LOCAL_SIZE * SCM_VARIABLE_SIZE; ++i) {
		t.locals[i] = 0;
	}
	strncpy(t.name, "THREAD", 16);
	t.conditionResult = false;
	t.conditionCount = 0;
	t.conditionAND = false;
	t.programCounter = start;
	t.baseAddress = start; /* Indicates where negative jumps should jump from */
	t.wakeCounter = 0;
	t.isMission = mission;
	t.finished = false;
	t.stackDepth = 0;
	_activeThreads.push_back(t);
}

SCMByte *ScriptMachine::getGlobals()
{
	return globalData.data();
}

void ScriptMachine::execute(float dt)
{
	int ms = dt * 1000.f;
	for(auto t = _activeThreads.begin(); t != _activeThreads.end(); ++t )
	{
		auto& thread = *t;
		executeThread( thread, ms );

		if( thread.finished ) {
			t = _activeThreads.erase( t );
        }
	}
}

