#define GLEW_STATIC
#include <GL/glew.h>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <unistd.h>
#include <iostream>
#include <fstream>
#include "../framework/rwbinarystream.h"

using RW::BSSectionHeader;
using RW::BSFrameList;
using RW::BSFrameListFrame;
using RW::BSClump;

using namespace RW;

sf::Window *window;

constexpr int WIDTH  = 800,
              HEIGHT = 600;

const char *vertexShaderSource = "#version 130\n"
"in vec3 position;"
"in vec2 texCoords;"
"out vec2 TexCoords;"
"uniform mat4 model;"
"uniform mat4 view;"
"uniform mat4 proj;"
"void main()"
"{"
"	TexCoords = texCoords;"
"	gl_Position = proj * model * vec4(position, 1.0);"
"}";
const char *fragmentShaderSource = "#version 130\n"
"in vec2 TexCoords;"
"uniform sampler2D texture;"
"void main()"
"{"
// "	gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);"
"	gl_FragColor = texture2D(texture, TexCoords);"
"}";

template<class T> T readStructure(char* data, size_t& dataI)
{
	size_t orgoff = dataI; dataI += sizeof(T);
	return *reinterpret_cast<T*>(data+orgoff);
}

BSSectionHeader readHeader(char* data, size_t& dataI)
{
	return readStructure<BSSectionHeader>(data, dataI);
}

GLuint compileShader(GLenum type, const char *source)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		GLint len;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
		GLchar *buffer = new GLchar[len];
		glGetShaderInfoLog(shader, len, NULL, buffer);

		std::cerr << "ERROR compiling shader: " << buffer << std::endl;
		delete[] buffer;
		exit(1);
	}

	return shader;
}

void dumpModelFile(char* data, size_t dataI)
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

void dumpTextureDictionary(char* data, size_t dataI)
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
		std::cout << " Width = " << std::dec << native.width << std::endl;
		std::cout << " Height = " << std::dec << native.height << std::endl;
		std::cout << " UV Wrap = " << std::hex << (native.wrapU+0) << "/" << (native.wrapV+0) << std::endl;
		std::cout << " Format = " << std::hex << (native.rasterformat) << std::endl;
		std::cout << " Name = " << std::string(native.diffuseName, 32) << std::endl;
		std::cout << " Alpha = " << std::string(native.alphaName, 32) << std::endl;
		
		dataI = basloc + textureHeader.size;
	}
}

void renderModel(char *data, size_t dataI)
{
	window = new sf::Window({WIDTH, HEIGHT}, "GTA Model Viewer", sf::Style::Close);

	glewExperimental = GL_TRUE;
	glewInit();

	readHeader(data, dataI); // Header
	readHeader(data, dataI); // ?
	readStructure<BSClump>(data, dataI); // Clump
	readHeader(data, dataI); // Frame header
	readHeader(data, dataI); // ?
	BSFrameList frames = readStructure<BSFrameList>(data, dataI); // Frames
	for (size_t i = 0; i < frames.numframes; ++i) 
		readStructure<BSFrameListFrame>(data, dataI);
	
	auto nextHeader = readHeader(data, dataI);
	while (nextHeader.id == RW::SID_Extension) {
		for (size_t i = 0; i < 2; ++i) {
			auto firstHeader = readHeader(data, dataI);
			dataI += firstHeader.size;
		}
		nextHeader = readHeader(data, dataI);
	}
	
	readHeader(data, dataI); // Structure Header..

	// OpenGL
	glClearColor(0.2, 0.2, 0.2, 1.0);
	glEnable(GL_DEPTH_TEST);
	// glDepthFunc(GL_GEQUAL);
	// glDisable(GL_CULL_FACE);

	GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
	GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);

	sf::Image uvgridTexture;
	uvgridTexture.loadFromFile("../datadump/uvgrid.jpg");
	// sf::Texture::bind(&uvgridTexture);

	GLuint VBO;
	GLuint EBO;
	GLuint textures[1];
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glGenTextures(1, textures);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, uvgridTexture.getSize().x, uvgridTexture.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, uvgridTexture.getPixelsPtr());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	auto geomlist = readStructure<BSGeometryList>(data, dataI);
	for (size_t i = 0; i < geomlist.numgeometry; ++i) {
		auto geomHeader = readHeader(data, dataI);
		size_t basedata = dataI;
		readHeader(data, dataI);
		auto geom = readStructure<BSGeometry>(data, dataI);
		
		if (geomHeader.versionid < 0x1003FFFF)
			auto colors = readStructure<BSGeometryColor>(data, dataI);
		
		if (geom.flags & BSGeometry::VertexColors) {
			for (size_t v = 0; v < geom.numverts; ++v) {
				readStructure<BSColor>(data, dataI);
			}
		}
		
		float *texcoords;
		size_t numTexcoords = 0;
		if (geom.flags & BSGeometry::TexCoords1 || geom.flags & BSGeometry::TexCoords2) {
			texcoords = new float[geom.numverts * 2];
			numTexcoords = geom.numverts * 2 * sizeof(float);

			for (size_t v = 0; v < geom.numverts; ++v) {
				auto coords = readStructure<BSGeometryUV>(data, dataI);

				texcoords[v*2]     = coords.u;
				texcoords[v*2 + 1] = coords.v;
			}
		}
		
		uint16_t indicies[geom.numtris * 3];
		for (int j = 0; j < geom.numtris; ++j) {
			auto tri = readStructure<BSGeometryTriangle>(data, dataI);

			indicies[j*3] = tri.first;
			indicies[j*3 + 1] = tri.second;
			indicies[j*3 + 2] = tri.third;
		}
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicies), indicies, GL_STATIC_DRAW);
		
		auto bounds = readStructure<BSGeometryBounds>(data,dataI);
		
		float *vertices = new float[geom.numverts * 3];
		size_t numVertices = geom.numverts * 3 * sizeof(float);
		for (size_t v = 0; v < geom.numverts; ++v) {
			auto p = readStructure<BSTVector3>(data, dataI);

			vertices[v*3]     = p.x;
			vertices[v*3 + 1] = p.y;
			vertices[v*3 + 2] = p.z;
		}

		float *normals;
		size_t numNormals = 0;
		if (geom.flags & BSGeometry::StoreNormals) {
			normals = new float[geom.numverts * 3];
			numNormals = geom.numverts * 3 * sizeof(float);

			for (size_t v = 0; v < geom.numverts; ++v) {
				auto p = readStructure<BSTVector3>(data, dataI);

				normals[v*3]     = p.x;
				normals[v*3 + 1] = p.y;
				normals[v*3 + 2] = p.z;
			}
		}
		
		// Jump to the start of the next geometry
		dataI = basedata + geomHeader.size;

		/** CHANGE THIS NUMBER TO SELECT A SPECIFIC MODEL! **/
		if (i <= 11)
			continue;

		// Buffer stuff
		size_t bufferSize = numVertices + numTexcoords + numNormals;
		std::cout << "BUFFER SIZE IS " << bufferSize << std::endl;

		glBufferData(GL_ARRAY_BUFFER, bufferSize, NULL, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, numVertices, vertices);
		if (numTexcoords > 0)
			glBufferSubData(GL_ARRAY_BUFFER, numVertices, numTexcoords, texcoords);
		if (numNormals > 0)
			glBufferSubData(GL_ARRAY_BUFFER, numVertices+numTexcoords, numNormals, normals);

		GLuint posAttrib = glGetAttribLocation(shaderProgram, "position");
		GLuint texAttrib = glGetAttribLocation(shaderProgram, "texCoords");
		glEnableVertexAttribArray(posAttrib);
		glEnableVertexAttribArray(texAttrib);
		glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 0, (void *) numVertices);

		GLuint uniModel = glGetUniformLocation(shaderProgram, "model");
		GLuint uniView = glGetUniformLocation(shaderProgram, "view");
		GLuint uniProj = glGetUniformLocation(shaderProgram, "proj");

		glm::mat4 proj = glm::perspective(80.f, (float) WIDTH/HEIGHT, 0.1f, 10.f);
		glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

		int j = 0;
		while (window->isOpen()) {
			glm::mat4 model;
			model = glm::translate(model, glm::vec3(0, 0, -0.5));
			model = glm::rotate(model, (float) j, glm::vec3(2, 1, 1));
			glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glDrawElements(GL_TRIANGLES, geom.numverts, GL_UNSIGNED_SHORT, NULL);
			
			window->display();
			j++;
		}

		break;
	}
}

bool loadFile(const char *filename, char **data)
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

	return true;
}

int main(int argc, char** argv)
{
	bool render = false;
	int c;
	while ((c = getopt (argc, argv, "r")) != -1) {
		switch (c) {
		case 'r':
			render = true;
			break;
		}
	}

	char *data;

	if (render) {
		if (loadFile(argv[2], &data)) {
			renderModel(data, 0);

			delete[] data;
		}
	} else {
		for (int i = 1; i < argc; ++i) {
			if ( ! loadFile(argv[i], &data))
				continue;

				std::string fname = argv[i];
				auto ext = fname.substr(fname.size()-3);
				
				if(ext == "dff" || ext == "DFF")
				{
					std::cout << "Dumping model file" << std::endl;
					dumpModelFile(data, 0);
				}
				else if(ext == "txd" || ext == "TXD")
				{
					std::cout << "Dumping texture archive" << std::endl;
					dumpTextureDictionary(data, 0);
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
