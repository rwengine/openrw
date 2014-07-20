#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>

#include <script/SCMFile.hpp>
#include <script/ScriptMachine.hpp>

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

void dumpOpcodes(SCMFile* scm, unsigned int offset, unsigned int size)
{
	std::cout << "Offs Opcd " << std::setw(FIELD_DESC_WIDTH) << std::left
			  << "Description" << "Parameters" << std::endl;

	for( unsigned int i = offset; i < offset+size; ) {
		SCMOpcode op = scm->read<SCMOpcode>(i) & ~SCM_CONDITIONAL_MASK;

		auto opit = knownOps.find( op );

		// If we don't know the size of the operator's parameters we can't jump over it.
		if( opit == knownOps.end() ) {
			throw IllegalInstruction(op, i);
		}

		std::cout << std::hex << std::setfill('0') << std::right <<
					 std::setw(4) << i << ":" <<
					 std::setw(4) << op <<
					 std::setw(FIELD_DESC_WIDTH) << std::setfill(' ') <<
					 std::left << opit->second.name << std::right;

		i += sizeof(SCMOpcode);

		bool hasMoreArgs = opit->second.parameters < 0;
		for( int p = 0; p < std::abs(opit->second.parameters) || hasMoreArgs; ++p ) {
			SCMByte datatype = scm->read<SCMByte>(i);

			auto typeit = typeData.find(static_cast<SCMType>(datatype));
			if( typeit == typeData.end()) {
				if( datatype < 0x06 ) {
					throw UnknownType(datatype, i);
				}
				else {
					datatype = TString;
				}
			}
			else {
				i += sizeof(SCMByte);
			}

			std::cout << " " << std::setfill('0') << std::setw(2) <<
						 static_cast<unsigned int>(datatype) << ": ";
			std::cout << std::setfill(' ') << std::setw(FIELD_PARAM_WIDTH);

			switch( datatype ) {
			case TInt32:
				std::cout << std::dec << scm->read<int32_t>(i);
				break;
			case TInt16:
				std::cout << std::dec << scm->read<int16_t>(i);
				break;
			case TGlobal:
			case TLocal:
				std::cout << std::hex << scm->read<int16_t>(i);
				break;
			case TInt8:
				std::cout << std::dec << static_cast<int>(scm->read<int8_t>(i));
				break;
			case TFloat16:
				std::cout << (float)scm->read<uint16_t>(i) / 16.f;
				break;
			case EndOfArgList:
				hasMoreArgs = false;
				break;
			case TString: {
				char strbuff[8];
				for(size_t c = 0; c < 8; ++c) {
					strbuff[c] = scm->read<char>(i++);
				}
				std::cout << strbuff << " ";
			}
				break;
			default:
				std::cout << "{unknown}";
				break;
			}

			if( typeit != typeData.end() ) {
				i += typeit->second.size;
			}
		}

		std::cout << std::endl;
	}
}

void loadKnownOps(const std::string& dbfile)
{
	std::ifstream dbstream(dbfile.c_str());

	if( !dbstream.is_open() ) {
		std::cerr << "Failed to open " << dbfile << std::endl;
		return;
	}

	while( ! dbstream.eof() ) {
		std::string line;
		std::getline(dbstream, line);
		auto fnws = line.find_first_not_of(" ");
		if( fnws == line.npos || line.at(fnws) == '#' ) continue;

		std::stringstream ss(line);

		std::string sec;

		std::getline(ss, sec, ',');

		SCMMicrocode m;

		uint16_t opcode = std::stoi(sec, 0, 16);

		std::getline(ss, m.name, ',');

		std::getline(ss, sec, ',');
		m.parameters = std::stoi(sec);

		std::getline(ss, sec, ',');
		uint16_t flags = std::stoi(sec);

		knownOps.insert({opcode, m});
	}

	std::cout << knownOps.size() << " known opcodes " << std::endl;
}

void readSCM(const std::string& scmname)
{
	std::ifstream scmfile(scmname.c_str());

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

		dumpOpcodes(&scm, scm.getCodeSection(), size);
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

	loadKnownOps("knownops.txt");

	readSCM(std::string(argv[1]));

	return 0;
}

void printUsage() {
	std::cout << "Usage:" << std::endl;
	std::cout << " scripttool scmfile" << std::endl;
}
