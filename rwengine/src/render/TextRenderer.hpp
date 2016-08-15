#ifndef RWENGINE_TEXTRENDERER_HPP
#define RWENGINE_TEXTRENDERER_HPP
#include <engine/GameData.hpp>
#include "OpenGLRenderer.hpp"

#define GAME_FONTS 3
#define GAME_GLYPHS 192

class GameRenderer;
/**
 * @brief Handles rendering of bitmap font textures.
 * 
 * In future, strings textures might be cached to improve performance, but
 * for now, we just render each glyph on it's own quad
 */
class TextRenderer
{
public:
	
	/**
	 * @todo Can this be merged with the gamestate text entries?
	 */
	struct TextInfo
	{
		enum TextAlignemnt
		{
			Left = 0,
			Right = 1,
			Center = 2
		};
		
		/// Font index @see TextRenderer::setFontTexture
		int font;
		/// Message to be displayed (including markup)
		std::string text;
		/// On screen position
		glm::vec2 screenPosition;
		/// font size
		float size;
		/// Base colour
		glm::u8vec3 baseColour;
		/// Background colour
		glm::u8vec4 backgroundColour;
		/// Horizontal Alignment
		TextAlignemnt align;
		/// Wrap width
		int wrapX;

		TextInfo();
	};
	
	/**
	 * Stores the information for kerning a glyph
	 */
	struct GlyphInfo
	{
		float widthFrac;
	};
	
	TextRenderer(GameRenderer* renderer);
	~TextRenderer();
	
	void setFontTexture( int index, const std::string& font );
	
	void renderText( const TextInfo& ti, bool forceColour = false );
	
private:
	std::string fonts[GAME_FONTS];
	GlyphInfo glyphData[GAME_GLYPHS];

	GameRenderer* renderer;
	Renderer::ShaderProgram* textShader;
	
	GeometryBuffer gb;
	DrawBuffer db;
};
#endif
