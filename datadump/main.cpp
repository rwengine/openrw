#include <iostream>
#include <fstream>
#include "../framework/rwbinarystream.h"

using RW::BSSectionHeader;
using RW::BSClump;

template<class T> T readStructure(char* data, size_t& dataI)
{
	size_t orgoff = dataI; dataI += sizeof(T);
	return *reinterpret_cast<T*>(data+orgoff);
}

int main(int argc, char** argv)
{
	for(int i = 1; i < argc; ++i)
	{
		std::ifstream dfile(argv[i]);
		if(!dfile.is_open()) {
			std::cerr << "Error opening file " << argv[i] << std::endl;
			continue;
		}
		
		dfile.seekg(0, std::ios_base::end);
		size_t length = dfile.tellg();
		dfile.seekg(0);
		char* data = new char[length];
		dfile.read(data, length);
		size_t dataI = 0;
		
		BSSectionHeader first = readStructure<BSSectionHeader>(data, dataI);
		
		std::cout << "ID = " << std::hex << (unsigned long)first.id << " (IsClump = " << (first.id == RW::SID_Clump) << ")" << std::endl;
		std::cout << "Size = " << std::dec << (unsigned long)first.size << " bytes" << std::endl;
		std::cout << "Version ID = " << std::hex << (unsigned long)first.versionid << std::endl;
		
		if(first.id == RW::SID_Clump) 
		{
			BSClump clump = readStructure<BSClump>(data, dataI);
			std::cout << " Clump Data" << std::endl;
			std::cout << "  Atomics = " << std::dec << (unsigned long)clump.numatomics << std::endl;
			std::cout << "  Lights = " << std::dec << (unsigned long)clump.numlights << std::endl;
			std::cout << "  Cameras = " << std::dec << (unsigned long)clump.numcameras << std::endl;
		}
		
		delete[] data;
	}
	
	return 0;
}
