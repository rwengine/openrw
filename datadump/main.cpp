#include <unistd.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <renderwure/BinaryStream.hpp>
#include <renderwure/loaders/LoaderCOL.hpp>
#include <renderwure/loaders/LoaderIFP.hpp>
#include <renderwure/loaders/rwbinarystream.h>

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

bool loadFile(const char *filename, char **data, size_t* size = nullptr)
{
	std::ifstream dfile(filename);
	if ( ! dfile.is_open()) {
		std::cerr << "Error opening file " << filename << std::endl;
		return false;
	}

	dfile.seekg(0, std::ios_base::end);
	size_t length = dfile.tellg();
	dfile.seekg(0);
	*data = new char[length];
	dfile.read(*data, length);
	
	if(size) *size = length;

	return true;
}

void dumpModelFile(char* data)
{
	BinaryStreamSection root(data);
	auto clump = root.readStructure<BSClump>();
	std::cout << "numatomics(" << clump.numatomics << ")" << std::endl;
	
	size_t dataI = 0;
	while(root.hasMoreData(dataI))
	{
		auto sec = root.getNextChildSection(dataI);
		
		switch(sec.header.id) 
		{
			case RW::SID_FrameList:
			{
				/*auto list =*/ sec.readStructure<BSFrameList>();
			}
			break;
			case RW::SID_GeometryList:
			{
				/*auto list =*/ sec.readStructure<BSGeometryList>();
				size_t gdataI = 0;
				while(sec.hasMoreData(gdataI))
				{
					auto item = sec.getNextChildSection(gdataI);
					if(item.header.id == RW::SID_Geometry)
					{
						auto geom = item.readStructure<BSGeometry>();
						std::cout << " verts(" << geom.numverts << ") tris(" << geom.numtris << ")" << std::endl;
					}
				}
			}
			break;
		}
	}
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
		std::cout << "  Position = " << frame.position.x << " " << frame.position.y << " "  << frame.position.z << std::endl;
// 		std::cout << "  Rotation = " << std::endl;
// 		std::cout << "   " << frame.rotation.a.x << " " << frame.rotation.a.y << " " << frame.rotation.a.z << std::endl;
// 		std::cout << "   " << frame.rotation.b.x << " " << frame.rotation.b.y << " " << frame.rotation.b.z << std::endl;
// 		std::cout << "   " << frame.rotation.c.x << " " << frame.rotation.c.y << " " << frame.rotation.c.z << std::endl;
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
			/*auto colors =*/ readStructure<BSGeometryColor>(data, dataI);
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
		
		/*auto materialListHeader =*/ readHeader(data, dataI);
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
				
				/*auto texture =*/ readStructure<BSTexture>(data, dataI);
				
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

void dumpTextureDictionary(char* data)
{
	BinaryStreamSection root(data);
	auto texdict = root.readStructure<BSTextureDictionary>();
	std::cout << std::dec << "tecount(" << texdict.numtextures << ")" << std::endl;
	
	size_t dataI = 0;
	while(root.hasMoreData(dataI))
	{
		BinaryStreamSection sec = root.getNextChildSection(dataI);
		
		if(sec.header.id == RW::SID_TextureNative)
		{
			auto texnative = sec.readStructure<BSTextureNative>();
			std::cout << "texture(\"" << texnative.diffuseName << "\")" << std::endl;
			std::cout << " size(" << std::dec << texnative.width << "x" << texnative.height << ") format(" << std::hex << texnative.rasterformat << ")" << std::endl;
			std::cout << " uvmode(" << std::hex << (texnative.wrapU+0) << "x" << (texnative.wrapV+0) << ") platform(" << std::hex << texnative.platform << ")" << std::endl;
		}
	}
	
	/*
		if(native.rasterformat & BSTextureNative::FORMAT_EXT_PAL8) 
		{
			// Read the palette
			auto palette = readStructure<BSPaletteData>(data, dataI);
			
			// We can just do this for the time being until we need to compress or something
			uint8_t fullcolor[native.width * native.height * 4];
			
			// Pretend the pallet is uint8
			
			for(size_t y = 0; y < native.height; ++y)
			{
				for(size_t x = 0; x < native.width; ++x)
				{
					size_t texI = ((y*native.width)+x) * 4;
					size_t palI = static_cast<size_t>(data[dataI+(y*native.width)+x])*4;
					fullcolor[texI+0] = palette.palette[palI+0];
					fullcolor[texI+1] = palette.palette[palI+1];
					fullcolor[texI+2] = palette.palette[palI+2];
					fullcolor[texI+3] = 255;
				}
			}
			
			GLuint texid = 0;
			glGenTextures(1, &texid);
			glBindTexture(GL_TEXTURE_2D, texid);
			// todo: not completely ignore everything the TXD says.
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, native.width, native.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, fullcolor);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			std::string name = std::string(native.diffuseName);
			loadedTextures.insert(std::make_pair(name, texid));
		};
	*/
}

void dumpBinaryStreamSection(BinaryStreamSection& parent, size_t depth, size_t maxdepth = 7)
{
	std::cout << std::string(depth, ' ') << "ID(" << std::hex << int(parent.header.id) << ") ";
	std::cout << "size(" << std::dec << int(parent.header.size) << "b) ";
	std::cout << "version(" << std::hex << int(parent.header.versionid) << ") ";
	
	size_t sectionOffset = 0, j = 0;
	bool readchildren = false;
	
	// Handle the specialised bits
	switch(parent.header.id)
	{
		case RW::SID_Struct:
		{
			std::cout << "structure";
		}
			break;
		case RW::SID_String:
		{
			std::cout << "string(\"" << std::string(parent.raw()) << "\")";
		}
			break;
		case RW::SID_GeometryList:
		{
			auto list = parent.readStructure<BSGeometryList>();
			std::cout << std::dec << "gcount(" << list.numgeometry << ")";
			readchildren = true;
		}
			break;
		case RW::SID_Geometry:
		{
			auto geometry = parent.readStructure<BSGeometry>();
			std::cout << std::dec << "tcount(" << geometry.numtris << ") vcount(" << geometry.numverts << ")";
			readchildren = true;
		}
			break;
		case RW::SID_MaterialList:
		{
			auto list = parent.readStructure<BSMaterialList>();
			std::cout << std::dec << "mcount(" << list.nummaterials << ")";
			readchildren = true;
		}
			break;
		case RW::SID_Material:
		{
			auto material = parent.readStructure<BSMaterial>();
			std::cout << std::dec << "tcount(" << material.numtextures << ")";
			readchildren = true;
		}
			break;
		case RW::SID_Texture:
		{
			/*auto texture =*/ parent.readStructure<BSTexture>();
			std::cout << "texture";
			readchildren = true;
		}
			break;
		case RW::SID_TextureNative:
		{
			auto texture = parent.readStructure<BSTextureNative>();
			std::cout << std::dec << "size(" << texture.width << "x" << texture.height << ") ";
			std::cout << " format(" << std::hex << texture.rasterformat << ")";
		}
			break;
		case RW::SID_NodeName:
		{
			std::string name(parent.raw(), parent.header.size);
			std::cout << " nodename(\"" << name << "\")";
		}
			break;
		case RW::SID_FrameList:
		{
			auto list = parent.readStructure<RW::BSFrameList>();
			size_t fdataI = sizeof(RW::BSSectionHeader) + sizeof(RW::BSFrameList);
			std::cout << " frames(" << std::dec << list.numframes << ") " << fdataI << " " << parent.offset;
			for(size_t f = 0; f < list.numframes; ++f) {
				auto frame = parent.readSubStructure<RW::BSFrameListFrame>(fdataI); fdataI += sizeof(RW::BSFrameListFrame);
				std::cout << std::endl << std::string(depth, ' ') << " index(" << frame.index << ") position (" << std::dec << frame.position.x << " " << frame.position.y << " " << frame.position.z << ")";
			}
			
			readchildren = true;
		}
			break;
		case RW::SID_Atomic: 
		{
			std::cout << " atomic";
			readchildren = true;
		}
			break;
		case RW::SID_Clump:
		case RW::SID_TextureDictionary:
		case RW::SID_Extension:
		{
			readchildren = true;
		}
			break;
		default:
		{
			std::cout << "Unknown Section";
		}
	};
	
	std::cout << std::endl;
	
	if(readchildren)
	{
		while(parent.hasMoreData(sectionOffset) && (j++) < 100 && depth < maxdepth) 
		{
			BinaryStreamSection sec = parent.getNextChildSection(sectionOffset);
			dumpBinaryStreamSection(sec, depth+1);
		}
	}
}

void dumpCollisionModel(char* data, size_t size)
{
	LoaderCOL coll;
	
	if(coll.load(data, size)) {
		std::cout << "Collision instances: " << coll.instances.size() << std::endl;
		for(auto it = coll.instances.begin(); it != coll.instances.end(); ++it) {
			std::cout << "Collision data (version " << it->version << ")" << std::endl;
			std::cout << " model: " << it->header.name << std::endl;
			std::cout << " model id: " << it->header.modelid << std::endl;
			std::cout << " spheres: " << it->header2.numspheres << std::endl;
			for( size_t b = 0; b < it->spheres.size(); ++b ) {
				auto& box = it->spheres[b];
				std::cout << "  radius: " << box.radius << " center: " << box.center.x << " " << box.center.y << " " << box.center.z << std::endl;
			}
			std::cout << " boxes: " << it->header2.numboxes << std::endl;
			for( size_t b = 0; b < it->boxes.size(); ++b ) {
				auto& box = it->boxes[b];
				std::cout << "  min: " << box.min.x << " " << box.min.y << " " << box.min.z << " max: " << box.max.x << " " << box.max.y << " " << box.max.z << std::endl;
			}
			std::cout << " faces: " << it->header2.numfaces << std::endl;
			std::cout << " verts: " << it->vertices.size() << std::endl;
			for( size_t v = 0; v < it->vertices.size(); ++v ) {
				std::cout << "  " << it->vertices[v].x << ", " << it->vertices[v].y << ", " << it->vertices[v].z << std::endl;
			}
		}
	}
}

void dumpGenericTree(char* data)
{
	BinaryStreamSection root(data);
	dumpBinaryStreamSection(root, 0);
}

void dumpAnimationFile(char* data)
{
    LoaderIFP loader;

   if(loader.loadFromMemory(data)) {
        std::cout << loader.animations.size() << " animations" << std::endl;

        for( auto it = loader.animations.begin();
             it != loader.animations.end(); ++it ) {
            Animation* a = it->second;
            std::cout << a->name << std::endl;
            std::cout << "  " << a->bones.size() << " bones" << std::endl;
            for( auto bit = a->bones.begin();
                 bit != a->bones.end(); ++bit ) {
                std::cout << "    " << bit->first << " (" << bit->second->frames.size() << " frames)" << std::endl;
                for( auto fit = bit->second->frames.begin();
                     fit != bit->second->frames.end();
                     ++fit ) {
                    std::cout << "      f " << fit->starttime << std::endl;
                }
            }
        }

    }
}

int main(int argc, char** argv)
{
	bool raw = false;
	int c;
	while ((c = getopt (argc, argv, "t")) != -1) {
		switch (c) {
		case 't':
			raw = true;
			break;
		}
	}

	char *data;
	size_t size;

	if(raw) {
		if(loadFile(argv[2], &data)) {
			dumpGenericTree(data);
		}
	} else {
		for (int i = 1; i < argc; ++i) {
			if ( ! loadFile(argv[i], &data, &size))
				continue;

			std::string fname = argv[i];
			auto ext = fname.substr(fname.size()-3);
			std::transform(ext.begin(), ext.begin(), ext.end(), ::tolower);
			
			if(ext == "dff")
			{
				std::cout << "Dumping model file" << std::endl;
				dumpModelFile(data);
			}
			else if(ext == "txd")
			{
				std::cout << "Dumping texture archive" << std::endl;
				dumpTextureDictionary(data);
			}
			else if(ext == "col")
			{
				std::cout << "Dumping Collsion file" << std::endl;
				dumpCollisionModel(data, size);
			}
            else if(ext == "ifp")
            {
                std::cout << "Dumping animation file" << std::endl;
                dumpAnimationFile(data);
            }
			else 
			{
				std::cout << "I'm not sure what that is" << std::endl;
			}
			
			delete[] data;
		}
	}
	
	return 0;
}
