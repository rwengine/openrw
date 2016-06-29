#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>

#include <script/SCMFile.hpp>
#include <script/ScriptMachine.hpp>
#include <script/ScriptDisassembly.hpp>
#include <script/modules/VMModule.hpp>
#include <script/modules/GameModule.hpp>
#include <script/modules/ObjectModule.hpp>

#define FIELD_DESC_WIDTH 30
#define FIELD_PARAM_WIDTH 8

void printUsage();

void dumpModels(SCMFile* file)
{
	std::cout << "model count: " << std::dec << file->getModels().size() << std::endl;
	int i = 0;
	for( auto& m : file->getModels() ) {
		std::cout << std::dec << (i++) << ": " << m << std::endl;
	}
}

void dumpCodeSizes(SCMFile* file)
{
	std::cout << "main size: " << std::hex <<
				 file->getMainSize() << std::endl;

	std::cout << "largest mission size: " << std::hex <<
				 file->getLargestMissionSize() << std::endl;

	std::cout << "mission count: " << std::dec <<
				 file->getMissionOffsets().size() << std::endl;

	int i = 0;
	for(auto& m : file->getMissionOffsets()) {
		std::cout << std::dec << (i++) << ": " << std::hex << m << std::endl;
	}
}

void dumpOpcodes(SCMFile* scm, SCMOpcodes* codes, unsigned int offset, unsigned int size)
{
	std::cout << "Offs Opcd " << std::setw(FIELD_DESC_WIDTH) << std::left
			  << "Description" << "Parameters" << std::endl;

	ScriptDisassembly disassembly(codes, scm);

	try
	{
		disassembly.disassemble(offset);
	}
	catch( IllegalInstruction& ex )
	{
		std::cerr << "Error during disassembly: \n"
			<< ex.what() << std::endl;
	}

	for( auto& inst : disassembly.getInstructions() )
	{
		ScriptFunctionMeta* code;
		if(! codes->findOpcode(inst.second.opcode, &code) )
		{
			std::cerr << "Invalid opcode in disassembly (" << inst.second.opcode << ")" << std::endl;
		}

		std::cout << std::hex << std::setfill('0') << std::right <<
		std::setw(4) << inst.first << ":" <<
		std::setw(4) << inst.second.opcode << " " <<
		std::setw(FIELD_DESC_WIDTH) << std::setfill(' ') <<
		std::left << code->signature << std::right << "(";

		for( SCMOpcodeParameter& param : inst.second.parameters )
		{
			switch( param.type )
			{
				case TInt8:
					std::cout << "  i8: " << param.integer;
					break;
				case TInt16:
					std::cout << "  i16: " << param.integer;
					break;
				case TInt32:
					std::cout << "  i32: " << param.integer;
					break;
				case TFloat16:
					std::cout << "  f16: " << param.real;
					break;
				case TString:
					std::cout << "  str: " << param.string;
					break;
				case TGlobal:
					std::cout << "  g: " << param.globalPtr;
					break;
				case TLocal:
					std::cout << "  l: " << param.globalPtr;
					break;
			}
		}

		std::cout << "  )\n";
	}
}

void disassemble(const std::string& scmname)
{
	std::ifstream scmfile(scmname.c_str(), std::ios_base::binary);

	if( !scmfile.is_open() ) {
		std::cerr << "Failed to open " << scmname << std::endl;
		return;
	}

	scmfile.seekg(0, std::ios_base::end);
	int size = scmfile.tellg();
	scmfile.seekg(0);

	SCMByte* byff = new SCMByte[size];
	scmfile.read(byff, size);
	SCMFile scm;
	scm.loadFile(byff, size);
	delete byff;

	try {
		std::cout << "section globals: " << std::hex <<
					 scm.getGlobalSection() << std::endl;
		std::cout << "section models: " << std::hex <<
					 scm.getModelSection() << std::endl;
		std::cout << "section sizes: " << std::hex <<
					 scm.getMissionSection() << std::endl;
		std::cout << "section main: " << std::hex <<
					 scm.getCodeSection() << std::endl;

		dumpModels(&scm);

		dumpCodeSizes(&scm);
		
		SCMOpcodes* opcodes = new SCMOpcodes;
		opcodes->modules.push_back(new VMModule);
		opcodes->modules.push_back(new GameModule);
		opcodes->modules.push_back(new ObjectModule);
		
		dumpOpcodes(&scm, opcodes, scm.getCodeSection(), size);
	}
	catch (SCMException& ex) {
		std::cerr << ex.what() << std::endl;
	}
}

int main(int argc, char** argv)
{
	if( argc < 2 ) {
		std::cerr << "Missing argument" << std::endl;
		printUsage();
		return 1;
	}

	disassemble(std::string(argv[1]));

	return 0;
}

void printUsage() {
	std::cout << "Usage:" << std::endl;
	std::cout << " scripttool scmfile" << std::endl;
}
