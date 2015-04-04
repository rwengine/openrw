#include <script/ScriptMachine.hpp>
#include <script/SCMFile.hpp>
#include <script/ScriptModule.hpp>

#if SCM_DEBUG_INSTRUCTIONS
#include <iostream>
#endif

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
	
	bool hasDebugging = !! bpHandler;
	
	while( t.wakeCounter == 0 ) {
		auto opcode = _file->read<SCMOpcode>(t.programCounter);
		auto opcorg = opcode;

		bool isNegatedConditional = ((opcode & SCM_NEGATE_CONDITIONAL_MASK) == SCM_NEGATE_CONDITIONAL_MASK);
		opcode = opcode & ~SCM_NEGATE_CONDITIONAL_MASK;

		ScriptFunctionMeta* foundcode;
		if( ! _ops->findOpcode(opcode, &foundcode) )
		{
			throw IllegalInstruction(opcode, t.programCounter, t.name);
		}
		ScriptFunctionMeta& code = *foundcode;
		
		// Keep the pc for the debugger
		auto pc = t.programCounter;
		t.programCounter += sizeof(SCMOpcode);

		SCMParams parameters;
		
		bool hasExtraParameters = code.arguments < 0;
		auto requiredParams = std::abs(code.arguments);

		for( int p = 0; p < requiredParams || hasExtraParameters; ++p ) {
			auto type_r = _file->read<SCMByte>(t.programCounter);
			auto type = static_cast<SCMType>(type_r);

			if( type_r > 42 ) {
				// for implicit strings, we need the byte we just read.
				type = TString;
			}
			else {
				t.programCounter += sizeof(SCMByte);
			}

			parameters.push_back(SCMOpcodeParameter { type, { 0 } });
			switch(type) {
			case EndOfArgList:
				hasExtraParameters = false;
				break;
			case TInt8:
				parameters.back().integer = _file->read<std::uint8_t>(t.programCounter);
				t.programCounter += sizeof(SCMByte);
				break;
			case TInt16:
				parameters.back().integer = _file->read<std::int16_t>(t.programCounter);
				t.programCounter += sizeof(SCMByte) * 2;
				break;
			case TGlobal: {
				auto v = _file->read<std::uint16_t>(t.programCounter);
				parameters.back().globalPtr = _globals + v * sizeof(SCMByte) * 4;
				t.programCounter += sizeof(SCMByte) * 2;
			}
				break;
			case TLocal: {
				auto v = _file->read<std::uint16_t>(t.programCounter);
				parameters.back().globalPtr = t.locals + v * sizeof(SCMByte) * 4;
				t.programCounter += sizeof(SCMByte) * 2;
			}
				break;
			case TInt32:
				parameters.back().integer = _file->read<std::uint32_t>(t.programCounter);
				t.programCounter += sizeof(SCMByte) * 4;
				break;
			case TString:
				std::copy(_file->data()+t.programCounter, _file->data()+t.programCounter+8,
						  parameters.back().string);
				t.programCounter += sizeof(SCMByte) * 8;
				break;
			case TFloat16:
				parameters.back().real = _file->read<std::int16_t>(t.programCounter) / 16.f;
				t.programCounter += sizeof(SCMByte) * 2;
				break;
			default:
				throw UnknownType(type, t.programCounter, t.name);
				break;
			};
		}

		if(code.function)
		{
			ScriptArguments sca(&parameters, &t, this);
			
			if( hasDebugging )
			{
				if( breakpoints.find(pc) != breakpoints.end() )
				{
					SCMBreakpoint bp;
					bp.pc = pc;
					bp.thread = &t;
					bp.vm = this;
					bp.function = &code;
					bp.args = &sca;
					bpHandler(bp);
				}
			}
			
			code.function(sca);
		}

		if(isNegatedConditional) {
			t.conditionResult = !t.conditionResult;
		}

		// Handle conditional results for IF statements.
		if( t.conditionCount > 0 && opcode != 0x00D6 ) /// @todo add conditional flag to opcodes instead of checking for 0x00D6
		{
			auto cI = --t.conditionCount;
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
	p.globalPtr = (t.locals + 16 * sizeof ( SCMByte ) * 4);
	*p.globalInteger += msPassed;
	p.globalPtr = (t.locals + 17 * sizeof ( SCMByte ) * 4);
	*p.globalInteger += msPassed;
	
	if( t.wakeCounter == -1 ) {
		t.wakeCounter = 0;
	}
}

ScriptMachine::ScriptMachine(GameWorld *world, SCMFile *file, SCMOpcodes *ops)
	: _file(file), _ops(ops), _world(world)
{
	startThread(0);
	auto globals = _file->getGlobalsSize() / 4;
	_globals = new SCMByte[globals * SCM_VARIABLE_SIZE];
	for(int i = 0; i < globals * SCM_VARIABLE_SIZE; ++i)
	{
		_globals[i] = 0;
	}
}

ScriptMachine::~ScriptMachine()
{
	delete _file;
	delete _ops;
	delete[] _globals;
}

void ScriptMachine::startThread(SCMThread::pc_t start, bool mission)
{
	SCMThread t;
	for(int i = 0; i < SCM_THREAD_LOCAL_SIZE; ++i) {
		t.locals[i] = 0;
	}
	t.name = "THREAD";
	t.conditionResult = false;
	t.conditionCount = 0;
	t.conditionAND = false;
	t.programCounter = start;
	t.baseAddress = start; /* Indicates where negative jumps should jump from */
	t.wakeCounter = 0;
	t.isMission = mission;
	t.finished = false;
	_activeThreads.push_back(t);
}

SCMByte *ScriptMachine::getGlobals()
{
	return _file->data() + _file->getGlobalSection();
}

void ScriptMachine::execute(float dt)
{
	int ms = dt * 1000.f;
	for(size_t ti = 0; ti < _activeThreads.size(); ++ti) {
		auto thread	= _activeThreads[ti];
		executeThread( thread, ms );

		if( thread.finished ) {
			_activeThreads.erase( _activeThreads.begin() + ti );
		}
		else {
			_activeThreads[ti] = thread;
		}
	}
}

void ScriptMachine::setBreakpointHandler(const ScriptMachine::BreakpointHandler& handler)
{
	bpHandler = handler;
}

void ScriptMachine::addBreakpoint(SCMThread::pc_t pc)
{
	breakpoints.insert(pc);
}

void ScriptMachine::removeBreakpoint(SCMThread::pc_t pc)
{
	breakpoints.erase(pc);
}


