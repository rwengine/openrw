#pragma once
#ifndef _TEXTURE_ATLAS_HPP_
#define _TEXTURE_ATLAS_HPP_
#include <GL/gl.h>

class TextureAtlas
{
	/**
	 * @brief width Width of the backing texture.
	 */
	size_t width;
	/**
	 * @brief height Height of the backing texture.
	 */
	size_t height;

	GLuint textureName;

	size_t X; // X edge of latest texture.
	size_t Y; // Y of current row.
	size_t rowHeight; // Maximum texture height for the current row.

public:

	TextureAtlas(size_t w, size_t h);

	~TextureAtlas();

	void packTexture(uint8_t* data, size_t w, size_t h, float& s, float& t, float& sX, float& sY);

	/**
	 * @brief canPack Returns true if enough space remains in the atlas
	 *  for the given texture sizes to be packed.
	 * @param w
	 * @param h
	 * @param n
	 * @return True on success, false on failure.
	 */
	bool canPack(size_t* w, size_t* h, size_t n) const;

};

#endif
