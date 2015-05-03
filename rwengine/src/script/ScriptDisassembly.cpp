#include <script/ScriptDisassembly.hpp>
#include <script/SCMFile.hpp>
#include <script/ScriptMachine.hpp>

ScriptDisassembly::ScriptDisassembly(SCMOpcodes* _codes, SCMFile* _scm)
 : codes(_codes), scm(_scm)
{

}

void ScriptDisassembly::disassemble(SCMAddress startAddress)
{
	for( SCMAddress a = startAddress; a < scm->getMainSize(); )
	{
		auto opcode = scm->read<SCMOpcode>(a);
		auto opcorg = opcode;
		
		bool isNegatedConditional = ((opcode & SCM_NEGATE_CONDITIONAL_MASK) == SCM_NEGATE_CONDITIONAL_MASK);
		opcode = opcode & ~SCM_NEGATE_CONDITIONAL_MASK;
		
		ScriptFunctionMeta* foundcode;
		if( ! codes->findOpcode(opcode, &foundcode) )
		{
			throw IllegalInstruction(opcode, a, "Disassembler");
		}
		ScriptFunctionMeta& code = *foundcode;
		
		SCMParams parameters;
		auto instructionAddress = a;
		a += sizeof(SCMOpcode);
		
		bool hasExtraParameters = code.arguments < 0;
		auto requiredParams = std::abs(code.arguments);
		
		for( int p = 0; p < requiredParams || hasExtraParameters; ++p ) {
			auto type_r = scm->read<SCMByte>(a);
			auto type = static_cast<SCMType>(type_r);
			
			if( type_r > 42 ) {
				// for implicit strings, we need the byte we just read.
				type = TString;
			}
			else {
				a += sizeof(SCMByte);
			}
			
			parameters.push_back(SCMOpcodeParameter { type, { 0 } });
			switch(type) {
				case EndOfArgList:
					hasExtraParameters = false;
					break;
				case TInt8:
					parameters.back().integer = scm->read<std::uint8_t>(a);
					a += sizeof(SCMByte);
					break;
				case TInt16:
					parameters.back().integer = scm->read<std::int16_t>(a);
					a += sizeof(SCMByte) * 2;
					break;
				case TGlobal: {
					auto v = scm->read<std::uint16_t>(a);
					parameters.back().globalPtr = (void*)v; //* SCM_VARIABLE_SIZE;
					a += sizeof(SCMByte) * 2;
				}
				break;
				case TLocal: {
					auto v = scm->read<std::uint16_t>(a);
					parameters.back().globalPtr = (void*)(v * SCM_VARIABLE_SIZE);
					a += sizeof(SCMByte) * 2;
				}
				break;
				case TInt32:
					parameters.back().integer = scm->read<std::uint32_t>(a);
					a += sizeof(SCMByte) * 4;
					break;
				case TString:
					std::copy(scm->data()+a, scm->data()+a+8,
							  parameters.back().string);
					a += sizeof(SCMByte) * 8;
					break;
				case TFloat16:
					parameters.back().real = scm->read<std::int16_t>(a) / 16.f;
					a += sizeof(SCMByte) * 2;
					break;
				default:
					throw UnknownType(type, a, "Disassembler");
					break;
			};
		}
		instructions[instructionAddress] = InstructionInfo { opcode, parameters };
	}
}
