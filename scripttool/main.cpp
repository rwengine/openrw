#include <iostream>
#include <sstream>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <map>

#define FIELD_DESC_WIDTH 30
#define FIELD_PARAM_WIDTH 8
#define CONDITIONAL_MASK 0xF000

void printUsage();

typedef uint16_t SCMOpcode;
typedef char SCMByte;

template<class T> T readFromSCM(SCMByte* scm, unsigned int offset)
{
	return *((T*)(scm+offset));
}

SCMOpcode readOpcode(SCMByte* scm, unsigned int offset)
{
	return readFromSCM<SCMOpcode>(scm, offset);
}

struct SCMException
{
	virtual ~SCMException() { }
	virtual std::string what() const = 0;
};

struct IllegalInstruction : SCMException
{
	SCMOpcode opcode;
	unsigned int offset;

	IllegalInstruction(SCMOpcode opcode, unsigned int offset)
		: opcode(opcode), offset(offset) { }

	std::string what() const {
		std::stringstream ss;
		ss << "Illegal Instruction " <<
			  std::setfill('0') << std::setw(4) << std::hex << opcode <<
			  " encountered at offset " <<
			  std::setfill('0') << std::setw(4) << std::hex << offset;
		return ss.str();
	}
};

struct UnknownType : SCMException
{
	SCMByte type;
	unsigned int offset;

	UnknownType(SCMByte type, unsigned int offset)
		: type(type), offset(offset) {}

	std::string what() const {
		std::stringstream ss;
		ss << "Unkown data type " <<
			  std::setfill('0') << std::hex << static_cast<unsigned int>(type) <<
			  " encountered at offset " <<
			  std::setfill('0') << std::hex << offset;
		return ss.str();
	}
};

struct SCMMicrocode {
	std::string name;
	int parameters;
};

typedef std::map<SCMOpcode, SCMMicrocode> SCMMicrocodeTable;

SCMMicrocodeTable knownOps;

enum SCMType {
	EndOfArgList = 0x00,
	TInt32       = 0x01,
	TGlobal      = 0x02,
	TLocal       = 0x03,
	TInt8        = 0x04,
	TInt16       = 0x05,
	TFloat16     = 0x06,
	TString      = 0x09,
};

struct SCMTypeInfo {
	uint8_t size;
};

typedef std::map<SCMType, SCMTypeInfo> SCMTypeInfoTable;

SCMTypeInfoTable typeData = {
	{TInt8,   {1}},
	{TInt16,  {2}},
	{TInt32,  {4}},
	{TInt8,   {1}},
	{TGlobal, {2}},
	{TLocal,  {2}},
	{TFloat16,{2}},
	{EndOfArgList, {0}},
};

void dumpSection(SCMByte* scm, unsigned int offset, unsigned int size)
{
	for( unsigned int i = offset; i < offset+size; ) {
		SCMOpcode op = readOpcode(scm, i) & ~CONDITIONAL_MASK;

		auto opit = knownOps.find( op );

		// If we don't know the size of the operator's parameters we can't jump over it.
		if( opit == knownOps.end() ) {
			throw IllegalInstruction(op, i);
		}

		std::cout << std::hex << std::setfill('0') << std::right <<
					 std::setw(4) << i << ":" <<
					 std::setw(4) << op <<
					 " " << std::setw(FIELD_DESC_WIDTH) << std::setfill(' ') <<
					 std::left << opit->second.name << std::right;

		i += sizeof(SCMOpcode);

		bool hasMoreArgs = opit->second.parameters < 0;
		for( int p = 0; p < std::abs(opit->second.parameters) || hasMoreArgs; ++p ) {
			SCMByte datatype = readFromSCM<SCMByte>(scm, i);

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
				std::cout << std::dec << readFromSCM<int32_t>(scm, i);
				break;
			case TInt16:
				std::cout << std::dec << readFromSCM<int16_t>(scm, i);
				break;
			case TGlobal:
			case TLocal:
				std::cout << std::hex << readFromSCM<int16_t>(scm, i);
				break;
			case TInt8:
				std::cout << std::dec << static_cast<int>(readFromSCM<int8_t>(scm, i));
				break;
			case TFloat16:
				std::cout << (float)readFromSCM<uint16_t>(scm, i) / 16.f;
				break;
			case EndOfArgList:
				hasMoreArgs = false;
				break;
			case TString: {
				char strbuff[8];
				for(size_t c = 0; c < 8; ++c) {
					strbuff[c] = readFromSCM<char>(scm, i++);
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

	SCMByte* scm = new SCMByte[size];
	scmfile.read(scm, size);

	int section_globals = 10;
	int section_models = 0;
	int section_sizes = 0;
	int section_main = 0;

	try {
		int i = 0;

		SCMOpcode op;
		SCMByte param;

		op = readOpcode(scm, i);
		i += sizeof(SCMOpcode);

		param = readFromSCM<SCMByte>(scm, i);
		i += sizeof(SCMByte);

		section_models = readFromSCM<int32_t>(scm, i);

		i = section_models;

		op = readOpcode(scm, i);
		i += sizeof(SCMOpcode);

		param = readFromSCM<SCMByte>(scm, i);
		i += sizeof(SCMByte);

		section_sizes = readFromSCM<int32_t>(scm, i);

		i = section_sizes;

		op = readOpcode(scm, i);
		i += sizeof(SCMOpcode);

		param = readFromSCM<SCMByte>(scm, i);
		i += sizeof(SCMByte);

		section_main = readFromSCM<int32_t>(scm, i);

		i = section_main;

		std::cout << "section_globals = " << section_globals << std::endl;
		std::cout << "section_models = " << section_models << std::endl;
		std::cout << "section_sizes = " << section_sizes << std::endl;
		std::cout << "section_main = " << section_main << std::endl;

		std::cout << "Offs Opcd " << std::setw(FIELD_DESC_WIDTH) << std::left
				  << "Description" << " Parameters" << std::endl;

		dumpSection(scm, i, size);
	}
	catch (SCMException& ex) {
		std::cerr << ex.what() << std::endl;
	}

	delete scm;
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
