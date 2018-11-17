#ifndef _RWENGINE_TEXTRENDERER_HPP_
#define _RWENGINE_TEXTRENDERER_HPP_

#include <array>
#include <memory>
#include <string>

#include <glm/glm.hpp>

#include <gl/DrawBuffer.hpp>
#include <gl/GeometryBuffer.hpp>

#include <fonts/GameTexts.hpp>
#include <render/OpenGLRenderer.hpp>

class GameRenderer;
/**
 * @brief Handles rendering of bitmap font textures.
 *
 * In future, strings textures might be cached to improve performance, but
 * for now, we just render each glyph on it's own quad
 */
class TextRenderer {
public:
    /**
     * @todo Can this be merged with the gamestate text entries?
     */
    struct TextInfo {
        enum class TextAlignment { Left = 0, Right = 1, Center = 2 };

        /// Font index @see TextRenderer::setFontTexture
        font_t font{FONT_PAGER};
        /// Message to be displayed (including markup)
        GameString text;
        /// On screen position
        glm::vec2 screenPosition{};
        /// font size
        float size{1.f};
        /// Base colour
        glm::u8vec3 baseColour{1.f, 1.f, 1.f};
        /// Background colour
        glm::u8vec4 backgroundColour{};
        /// Horizontal Alignment
        TextAlignment align = TextAlignment::Left;
        /// Wrap width
        int wrapX{0};

        TextInfo() = default;
    };

    /**
     * Stores the information for kerning a glyph
     */
    struct GlyphInfo {
        float widthFrac;
    };

    TextRenderer(GameRenderer* renderer);
    ~TextRenderer() = default;

    void setFontTexture(font_t font, const std::string& textureName);

    void renderText(const TextInfo& ti, bool forceColour = false);

private:
    class FontMetaData {
    public:
        FontMetaData() = default;
        template<size_t N>
        FontMetaData(const std::string& textureName,
                     const std::array<std::uint8_t, N>& glyphWidths,
                     const glm::u32vec2& textureSize,
                     const glm::u8vec2& glyphOffset,
                     const std::uint8_t monoWidth)
            : textureName(textureName)
            , glyphWidths(glyphWidths.cbegin(), glyphWidths.cend())
            , textureSize(textureSize)
            , glyphOffset(glyphOffset)
            , monoWidth(monoWidth) {
        }
        std::string textureName;
        std::vector<std::uint8_t> glyphWidths;
        glm::u32vec2 textureSize;
        glm::u8vec2 glyphOffset;
        std::uint8_t monoWidth;
    };

    std::array<FontMetaData, FONTS_COUNT> fonts;

    GameRenderer* renderer;
    std::unique_ptr<Renderer::ShaderProgram> textShader;

    GeometryBuffer gb;
    DrawBuffer db;
};
#endif
