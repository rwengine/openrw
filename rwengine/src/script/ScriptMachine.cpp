#include <script/ScriptMachine.hpp>
#include <script/SCMFile.hpp>

void ScriptMachine::executeThread(SCMThread &t, int msPassed)
{
	if( t.wakeCounter > 0 ) {
		t.wakeCounter = std::max( t.wakeCounter - msPassed, 0 );
	}
	if( t.wakeCounter > 0 ) return;

	while( t.wakeCounter == 0 ) {
		auto opcode = _file->read<SCMOpcode>(t.programCounter);
		auto it = _ops->codes.find(opcode);
		if( it == _ops->codes.end() ) throw IllegalInstruction(opcode, t.programCounter, t.name);
		t.programCounter += sizeof(SCMOpcode);

		SCMMicrocode& code = it->second;

		SCMParams parameters;

		for( int p = 0; p < code.parameters; ++p ) {
			auto type_r = _file->read<SCMByte>(t.programCounter);
			auto type = static_cast<SCMType>(type_r);

			if( type_r > 42 ) {
				// for implicit strings, we need the byte we just read.
				type = TString;
			}
			else {
				t.programCounter += sizeof(SCMByte);
			}

			parameters.push_back(SCMOpcodeParameter { type, 0 });
			switch(type) {
			case TInt8:
				parameters.back().integer = _file->read<std::uint8_t>(t.programCounter);
				t.programCounter += sizeof(SCMByte);
				break;
			case TInt16:
				parameters.back().integer = _file->read<std::uint16_t>(t.programCounter);
				t.programCounter += sizeof(SCMByte) * 2;
				break;
			case TGlobal: {
				auto v = _file->read<std::uint16_t>(t.programCounter);
				parameters.back().globalPtr = getGlobals() + v;
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
				parameters.back().real = (float)(_file->read<std::uint16_t>(t.programCounter)) / 16.f;
				t.programCounter += sizeof(SCMByte) * 2;
				break;
			default:
				throw UnknownType(type, t.programCounter, t.name);
				break;
			};
		}

		code.func(this, &t, &parameters);
	}

	if( t.wakeCounter == -1 ) {
		t.wakeCounter = 0;
	}
}

ScriptMachine::ScriptMachine(SCMFile *file, SCMOpcodes *ops)
	: _file(file), _ops(ops)
{
	startThread(0);
}

ScriptMachine::~ScriptMachine()
{
	delete _file;
	delete _ops;
}

void ScriptMachine::startThread(SCMThread::pc_t start)
{
	SCMThread t;
	for(int i = 0; i < SCM_THREAD_LOCAL_SIZE; ++i) {
		t.locals[i] = 0;
	}
	t.name = "THREAD";
	t.programCounter = start;
	t.wakeCounter = 0;
	_activeThreads.push_back(t);
}

SCMByte *ScriptMachine::getGlobals()
{
	return _file->data() + _file->getGlobalSection();
}

void ScriptMachine::execute(float dt)
{
	int ms = dt * 1000.f;
	for(auto& thread : _activeThreads) {
		executeThread( thread, ms );
	}
}
