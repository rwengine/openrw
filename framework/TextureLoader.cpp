#include "TextureLoader.h"

#include <fstream>
#include <iostream>

bool TextureLoader::loadFromFile(std::string filename)
{
	std::ifstream dfile(filename);
	if ( ! dfile.is_open()) {
		std::cerr << "Error opening file " << filename << std::endl;
		return false;
	}

	dfile.seekg(0, std::ios_base::end);
	size_t length = dfile.tellg();
	dfile.seekg(0);
	char *data = new char[length];
	dfile.read(data, length);

	return loadFromMemory(data);
}

bool TextureLoader::loadFromMemory(char *data)
{
	RW::BinaryStreamSection root(data);
	auto texDict = root.readStructure<RW::BSTextureDictionary>();

	size_t rootI = 0;
	while (root.hasMoreData(rootI)) {
		auto rootSection = root.getNextChildSection(rootI);

		if (rootSection.header.id != RW::SID_TextureNative)
			continue;

		auto texNative = rootSection.readStructure<RW::BSTextureNative>();
		if ( ! (texNative.rasterformat & RW::BSTextureNative::FORMAT_EXT_PAL8))
			continue;

		auto palette = rootSection.readSubStructure<RW::BSPaletteData>(sizeof(RW::BSTextureNative));
		auto coldata = rootSection.raw() + sizeof(RW::BSTextureNative) + sizeof(RW::BSPaletteData);

		uint8_t fullColor[texNative.width * texNative.height * 4];

		for (size_t y = 0; y < texNative.height; y++) {
			for (size_t x = 0; x < texNative.width; x++) {
				size_t texI = ((y * texNative.width) + x) * 4;
				size_t palI = 4 * static_cast<size_t>(
					coldata[(y * texNative.width) + x]
				);

				fullColor[texI+0] = palette.palette[palI+2];
				fullColor[texI+1] = palette.palette[palI+1];
				fullColor[texI+2] = palette.palette[palI+0];
				fullColor[texI+3] = palette.palette[palI+3];
			}
		}

		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		// todo: not completely ignore everything the TXD says.
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGBA,
			texNative.width, texNative.height, 0,
			GL_BGRA, GL_UNSIGNED_BYTE, fullColor
		);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		std::string name = std::string(texNative.diffuseName);

		textures[name] = texture;

		std::cout << "Loaded texture '" << name << "'" << std::endl;
	}

	return true;
}

void TextureLoader::bindTexture(std::string texture)
{
	if (textures.find(texture) == textures.end()) {
		std::cerr << "Could not find nor bind texture '" << texture << "'" << std::endl;
		return;
	}

	glBindTexture(GL_TEXTURE_2D, textures[texture]);
}
