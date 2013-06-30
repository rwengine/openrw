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
	
	auto nextHeader = readHeader(data, dataI);
	while(nextHeader.id == RW::SID_Extension) 
	{
		for(size_t i = 0; i < 2; ++i) {
			auto firstHeader = readHeader(data, dataI);
			if(firstHeader.id == RW::SID_NodeName) 
			{
				std::cout << "  Name = " << std::string(data+dataI, firstHeader.size) << std::endl;
			}
			else if(firstHeader.id == RW::SID_HAnimPLG)
			{
				std::cout << "  Bone Information Present" << std::endl;
			}
			dataI += firstHeader.size;
		}
		nextHeader = readHeader(data, dataI);
	}
	
	readHeader(data, dataI); // Structure Header..
	
	auto geomlist = readStructure<BSGeometryList>(data, dataI);
	std::cout << " Geometry List Data" << std::endl;
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
		
		if(geomHeader.versionid < 0x1003FFFF) 
		{
			std::cout << "  Some extra colour info" << std::endl;
			auto colors = readStructure<BSGeometryColor>(data, dataI);
		}
		
		if(geom.flags & BSGeometry::VertexColors)
		{
			std::cout << "  Vertex Colours Present" << std::endl;
			for(size_t v = 0; v < geom.numverts; ++v) 
			{
				std::cout << "  " << v << ": " << static_cast<unsigned long>(readStructure<BSColor>(data, dataI)) << std::endl;
			}
		}
		
		if(geom.flags & BSGeometry::TexCoords1 || geom.flags & BSGeometry::TexCoords2)
		{
			std::cout << "  UV Coords Present" << std::endl;
			for(size_t v = 0; v < geom.numverts; ++v) 
			{
				auto coords = readStructure<BSGeometryUV>(data, dataI);
				std::cout << "  " << v << ": U" << coords.u << " V" << coords.v <<  std::endl;
			}
		}
		
		for(int j = 0; j < geom.numtris; ++j) 
		{
			auto tri = readStructure<BSGeometryTriangle>(data, dataI);
			std::cout << "  Triangle " << std::dec 
				<< static_cast<unsigned long>(tri.first) << " " 
				<< static_cast<unsigned long>(tri.second) << " " 
				<< static_cast<unsigned long>(tri.third) << " " 
				<< "A: " << static_cast<unsigned long>(tri.attrib) << std::endl; 
		}
		
		auto bounds = readStructure<BSGeometryBounds>(data,dataI);
		std::cout << "  Bounding Radius = " << bounds.radius << std::endl;
		
		for(size_t v = 0; v < geom.numverts; ++v) 
		{
			auto p = readStructure<BSTVector3>(data, dataI);
			std::cout << "  v " << p.x << " " << p.y << " " << p.z << std::endl;
		}
		
		if(geom.flags & BSGeometry::StoreNormals) 
		{
			std::cout << "  Vertex Normals present" << std::endl;
			for(size_t v = 0; v < geom.numverts; ++v) 
			{
				auto p = readStructure<BSTVector3>(data, dataI);
				std::cout << "  n " << p.x << " " << p.y << " " << p.z << std::endl;
			}
		}
		
		auto materialListHeader = readHeader(data, dataI);
		readHeader(data, dataI); // Ignore the structure header..
		
		auto materialList = readStructure<BSMaterialList>(data, dataI);
		std::cout << " Material List Data" << std::endl;
		std::cout << "  Materials = " << materialList.nummaterials << std::endl;
		
		// Skip over the per-material byte values that I don't know what do.
		dataI += sizeof(uint32_t) * materialList.nummaterials;
		
		for(size_t m = 0; m < materialList.nummaterials; ++m)
		{
			auto materialHeader = readHeader(data, dataI);
			size_t secbase = dataI;
			readHeader(data, dataI);
			
			auto material = readStructure<BSMaterial>(data, dataI);
			std::cout << " Material Data" << std::endl;
			std::cout << "  Textures = " << std::dec << material.numtextures << std::endl;
			std::cout << "  Color = 0x" << std::hex << material.color << std::endl;
			
			for(size_t t = 0; t < material.numtextures; ++t) 
			{
				auto textureHeader = readHeader(data, dataI);
				size_t texsecbase = dataI;
				readHeader(data, dataI);
				
				auto texture = readStructure<BSTexture>(data, dataI);
				
				auto nameHeader = readHeader(data, dataI);
				std::string textureName(data+dataI, nameHeader.size);
				dataI += nameHeader.size;
				auto alphaHeader = readHeader(data, dataI);
				std::string alphaName(data+dataI, alphaHeader.size);
				
				std::cout << " Texture Data" << std::endl;
				std::cout << "  Name = " << textureName << std::endl;
				std::cout << "  Alpha = " << alphaName << std::endl;
				
				dataI = texsecbase + textureHeader.size;
			}
			
			dataI = secbase + materialHeader.size;
		}
		
		// Jump to the start of the next geometry
		dataI = basedata + geomHeader.size;
	}
}

void dumpTextureDictionary(char* data, size_t& dataI)
{
	auto header = readHeader(data, dataI);
	
	std::cout << "ID = " << std::hex << (unsigned long)header.id << " (IsTextureDirectory = " << (header.id == RW::SID_TextureDictionary) << ")" << std::endl;
	std::cout << "Size = " << std::dec << (unsigned long)header.size << " bytes" << std::endl;
	std::cout << "Version ID = " << std::hex << (unsigned long)header.versionid << std::endl;
	
	readHeader(data, dataI);
	
	auto dir = readStructure<BSTextureDictionary>(data, dataI);
	std::cout << "Texture Count = " << dir.numtextures << std::endl;
	
	for(size_t t = 0; t < dir.numtextures; ++t) 
	{
		auto textureHeader = readHeader(data, dataI);
		auto basloc = dataI;
		
		readHeader(data, dataI);
		
		auto native = readStructure<BSTextureNative>(data, dataI);
		std::cout << "Texture Info" << std::endl;
		std::cout << " Platform = " << std::hex << (native.platform) << std::endl;
		std::cout << " Width = " << std::dec << native.width << std::endl;
		std::cout << " Height = " << std::dec << native.height << std::endl;
		std::cout << " UV Wrap = " << std::hex << (native.wrapU+0) << "/" << (native.wrapV+0) << std::endl;
		std::cout << " Format = " << std::hex << (native.rasterformat) << std::endl;
		std::cout << " Name = " << std::string(native.diffuseName, 32) << std::endl;
		std::cout << " Alpha = " << std::string(native.alphaName, 32) << std::endl;
		std::cout << " DXT = " << std::hex << (native.dxttype+0) << std::endl;
		
		if(native.rasterformat & BSTextureNative::FORMAT_EXT_PAL8) 
		{
			// Read the palette
			auto palette = readStructure<BSPaletteData>(data, dataI);
			
			// We can just do this for the time being until we need to compress or something
			uint32_t fullcolor[native.width * native.height];
			
			for(size_t y = 0; y < native.height; ++y)
			{
				for(size_t x = 0; x < native.width; ++x)
				{
					fullcolor[(y*native.width)+x] = palette.palette[static_cast<size_t>(data[dataI+(y*native.width)+x])];
				}
			}
		};
		
		dataI = basloc + textureHeader.size;
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
		
		std::string fname = argv[i];
		auto ext = fname.substr(fname.size()-3);
		
		if(ext == "dff" || ext == "DFF")
		{
			std::cout << "Dumping model file" << std::endl;
			dumpModelFile(data, dataI);
		}
		else if(ext == "txd" || ext == "TXD")
		{
			std::cout << "Dumping texture archive" << std::endl;
			dumpTextureDictionary(data, dataI);
		}
		else 
		{
			std::cout << "I'm not sure what that is" << std::endl;
		}
		
		delete[] data;
	}
	
	return 0;
}
