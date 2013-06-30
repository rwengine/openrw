#include <iostream>
#include <fstream>
#include "../framework/rwbinarystream.h"

using RW::BSSectionHeader;
using RW::BSFrameList;
using RW::BSFrameListFrame;
using RW::BSClump;

using namespace RW;

template<class T> T readStructure(char* data, size_t& dataI)
{
	size_t orgoff = dataI; dataI += sizeof(T);
	return *reinterpret_cast<T*>(data+orgoff);
}

BSSectionHeader readHeader(char* data, size_t& dataI)
{
	return readStructure<BSSectionHeader>(data, dataI);
}

void dumpModelFile(char* data, size_t& dataI)
{
	auto header = readHeader(data, dataI);
	
	std::cout << "ID = " << std::hex << (unsigned long)header.id << " (IsClump = " << (header.id == RW::SID_Clump) << ")" << std::endl;
	std::cout << "Size = " << std::dec << (unsigned long)header.size << " bytes" << std::endl;
	std::cout << "Version ID = " << std::hex << (unsigned long)header.versionid << std::endl;
	
	readHeader(data, dataI);
	
	auto clump = readStructure<BSClump>(data, dataI);
	std::cout << " Clump Data" << std::endl;
	std::cout << "  Atomics = " << std::dec << (unsigned long)clump.numatomics << std::endl;
	
	auto frameheader = readHeader(data, dataI);
	std::cout << "ID = " << std::hex << (unsigned long)frameheader.id << " (IsFrameList = " << (frameheader.id == RW::SID_FrameList) << ")" << std::endl;
	
	readHeader(data, dataI);
	
	BSFrameList frames = readStructure<BSFrameList>(data, dataI);
	std::cout << " Frame List Data" << std::endl;
	std::cout << "  Frames = " << std::dec << (unsigned long)frames.numframes << std::endl;
	
	for(size_t i = 0; i < frames.numframes; ++i) 
	{
		BSFrameListFrame frame = readStructure<BSFrameListFrame>(data, dataI);
		std::cout << " Frame Data" << std::endl;
		std::cout << "  Index = " << std::dec << (unsigned long)frame.index << std::endl;
		std::cout << "  Position = " << frame.postiion.x << " " << frame.postiion.y << " "  << frame.postiion.z << std::endl;
		std::cout << "  Rotation = " << std::endl;
		std::cout << "   " << frame.rotation.a.x << " " << frame.rotation.a.y << " " << frame.rotation.a.z << std::endl;
		std::cout << "   " << frame.rotation.b.x << " " << frame.rotation.b.y << " " << frame.rotation.b.z << std::endl;
		std::cout << "   " << frame.rotation.c.x << " " << frame.rotation.c.y << " " << frame.rotation.c.z << std::endl;
	}
	
	// Skip over the Frame extentions we can't parse.
	auto nextHeader = readHeader(data, dataI);
	while(nextHeader.id == RW::SID_Extension) 
	{
		dataI += nextHeader.size;
		nextHeader = readHeader(data, dataI);
	}
	
	readHeader(data, dataI); // Structure Header..
	
	auto geomlist = readStructure<BSGeometryList>(data, dataI);
	std::cout << " Geometry List Data" << std::endl;
	std::cout << std::hex << dataI << std::endl;
	std::cout << "  Geometries = " << std::dec << geomlist.numgeometry << std::endl;
	for(size_t i = 0; i < geomlist.numgeometry; ++i) 
	{
		auto geomHeader = readHeader(data, dataI);
		size_t basedata = dataI;
		readHeader(data, dataI);
		auto geom = readStructure<BSGeometry>(data, dataI);
		std::cout << " Geometry Data" << std::endl;
		std::cout << "  Flags = " << std::hex << static_cast<unsigned long>(geom.flags) << std::endl;
		std::cout << "  UV Sets = " << std::dec << static_cast<unsigned long>(geom.numuvs) << std::endl;
		std::cout << "  Flags = " << std::hex << static_cast<unsigned long>(geom.geomflags) << std::endl;
		std::cout << "  Triangles = " << std::dec << static_cast<unsigned long>(geom.numtris) << std::endl;
		std::cout << "  Verticies = " << static_cast<unsigned long>(geom.numverts) << std::endl;
		std::cout << "  Frames = " << static_cast<unsigned long>(geom.numframes) << std::endl;
		
		// Jump to the start of the next geometry
		dataI = basedata + geomHeader.size;
	}
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
		
		dumpModelFile(data, dataI);
		
		delete[] data;
	}
	
	return 0;
}
