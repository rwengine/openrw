#include "render/TextRenderer.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <vector>

#include <gl/gl_core_3_3.h>

#include "engine/GameData.hpp"
#include "render/GameRenderer.hpp"

namespace {

unsigned charToIndex(std::uint16_t g) {
    // Correct for the default font maps
    /// @todo confirm for JA / RU font maps
    return g - 32;
}

glm::vec4 indexToTexCoord(int index, const glm::u32vec2 &textureSize, const glm::u8vec2 &glyphOffset) {
    constexpr unsigned TEXTURE_COLUMNS = 16;
    const float x = index % TEXTURE_COLUMNS;
    const float y = index / TEXTURE_COLUMNS;
    // Add offset to avoid 'leakage' between adjacent glyphs
    float s = (x * glyphOffset.x + 0.5f) / textureSize.x;
    float t = (y * glyphOffset.y + 0.5f) / textureSize.y;
    float p = ((x + 1) * glyphOffset.x - 1.5f) / textureSize.x;
    float q = ((y + 1) * glyphOffset.y - 1.5f) / textureSize.y;
    return glm::vec4(s, t, p, q);
}

const char* TextVertexShader = R"(
#version 330

layout(location = 0) in vec2 position;
layout(location = 3) in vec2 texcoord;
layout(location = 2) in vec3 colour;
out vec2 TexCoord;
out vec3 Colour;

uniform mat4 proj;
uniform vec2 alignment;

void main()
{
    gl_Position = proj * vec4(alignment + position, 0.0, 1.0);
    TexCoord = texcoord;
    Colour = colour;
})";

const char* TextFragmentShader = R"(
#version 330

in vec2 TexCoord;
in vec3 Colour;
uniform vec4 colour;
uniform sampler2D fontTexture;
out vec4 outColour;

void main()
{
    float a = texture(fontTexture, TexCoord).a;
    outColour = vec4(Colour, a);
})";


constexpr size_t GLYPHS_NB = 193;
using FontWidthLut = std::array<std::uint8_t, GLYPHS_NB>;

constexpr std::array<std::uint8_t, 193> fontWidthsPager = {
     3,  3,  6,  8,  6, 10,  8,  3,  5,  5,  7,  0,  3,  7,  3,  0, //  1
     6,  4,  6,  6,  7,  6,  6,  6,  6,  6,  3,  0,  0,  0,  0,  6, //  2
     0,  6,  6,  6,  6,  6,  6,  6,  6,  3,  6,  6,  5,  8,  7,  6, //  3
     6,  7,  6,  6,  5,  6,  6,  8,  6,  7,  7,  0,  0,  0,  0,  0, //  4
     0,  6,  6,  6,  6,  6,  5,  6,  6,  3,  4,  6,  3,  9,  6,  6, //  5
     6,  6,  5,  6,  5,  6,  6,  8,  6,  6,  5,  0,  0,  0,  0,  0, //  6
     6,  6,  6,  6,  8,  6,  6,  6,  6,  6,  5,  5,  6,  6,  6,  6, //  7
     6,  6,  6,  6,  6,  6,  7,  6,  6,  6,  6,  9,  6,  6,  6,  6, //  8
     6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  7,  6,  6, //  9
     3,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8, // 10
     8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8, // 11
     8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8, // 12
     8,
};

constexpr std::array<std::uint8_t, 193> fontWidthsPriceDown = {
    11, 13, 30, 27, 20, 24, 22, 12, 14, 14,  0, 26,  9, 14,  9, 26, //  1
    20, 19, 20, 20, 22, 20, 20, 19, 20, 20, 13, 29, 24, 29, 24, 20, //  2
    27, 20, 20, 20, 20, 20, 17, 20, 20, 10, 20, 20, 15, 30, 20, 20, //  3
    20, 20, 20, 20, 22, 20, 22, 32, 20, 20, 19, 27, 20, 32, 23, 13, //  4
    27, 21, 21, 21, 21, 21, 18, 22, 21, 12, 20, 22, 17, 30, 22, 21, //  5
    21, 21, 21, 22, 21, 21, 21, 29, 19, 23, 21, 28, 25,  0,  0,  0, //  6
    20, 20, 20, 20, 30, 20, 20, 20, 20, 20, 10, 10, 10, 10, 21, 21, //  7
    21, 21, 20, 20, 20, 20, 21, 21, 21, 21, 21, 32, 23, 21, 21, 21, //  8
    21, 12, 12, 12, 12, 21, 21, 21, 21, 21, 21, 21, 21, 20, 20, 20, //  9
    13, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, // 10
    19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, // 11
    19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, // 12
    16,
};

constexpr std::array<std::uint8_t, 193> fontWidthsArial = {
    27, 25, 55, 43, 47, 65, 53, 19, 29, 31, 21, 45, 23, 35, 27, 29, //  1
    47, 33, 45, 43, 49, 47, 47, 41, 47, 45, 25, 23, 53, 43, 53, 39, //  2
    61, 53, 51, 47, 49, 45, 43, 49, 53, 23, 41, 53, 45, 59, 53, 51, //  3
    47, 51, 49, 49, 45, 51, 49, 59, 59, 47, 51, 31, 27, 31, 29, 27, //  4
    19, 43, 45, 43, 43, 45, 27, 45, 43, 21, 33, 45, 23, 65, 43, 43, //  5
    47, 45, 33, 41, 29, 43, 41, 61, 51, 43, 43, 67, 53, 67, 67, 71, //  6
    53, 53, 53, 53, 65, 49, 45, 45, 45, 45, 23, 23, 23, 23, 51, 51, //  7
    51, 51, 51, 51, 51, 51, 51, 43, 43, 43, 43, 65, 43, 45, 45, 45, //  8
    45, 21, 21, 21, 21, 43, 43, 43, 43, 43, 43, 43, 43, 53, 43, 39, //  9
    25, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, // 10
    19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 11, 19, 19, // 11
    19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, // 12
    19,
};

}

struct TextVertex {
    glm::vec2 position;
    glm::vec2 texcoord;
    glm::vec3 colour;

    TextVertex(glm::vec2 _position, glm::vec2 _texcoord, glm::vec3 _colour)
        : position(_position)
        , texcoord(_texcoord)
        , colour(_colour) {
    }

    TextVertex() = default;

    static const AttributeList vertex_attributes() {
        return {
            {ATRS_Position, 2, sizeof(TextVertex), 0ul},
            {ATRS_TexCoord, 2, sizeof(TextVertex), 0ul + sizeof(glm::vec2)},
            {ATRS_Colour, 3, sizeof(TextVertex), 0ul + sizeof(glm::vec2) * 2},
        };
    }
};

TextRenderer::TextRenderer(GameRenderer* renderer) : renderer(renderer) {
    textShader = renderer->getRenderer()->createShader(TextVertexShader,
                                                       TextFragmentShader);
}

void TextRenderer::setFontTexture(font_t font, const std::string& textureName) {
    auto ftexture = renderer->getData()->findSlotTexture("fonts", textureName);
    const glm::u32vec2 textureSize = ftexture->getSize();
    glm::u8vec2 glyphOffset{textureSize.x / 16, textureSize.x / 16};
    if (font != FONT_PAGER) {
        glyphOffset.y += glyphOffset.y / 4;
    }
    const FontWidthLut *glyphWidths;
    switch (font) {
    case FONT_PAGER:
        glyphWidths = &fontWidthsPager;
        break;
    case FONT_PRICEDOWN:
        glyphWidths = &fontWidthsPriceDown;
        break;
    case FONT_ARIAL:
        glyphWidths = &fontWidthsArial;
        break;
    default:
        throw std::runtime_error("Illegal font");
    }
    std::uint8_t monoWidth = 0;
    if (font == FONT_PAGER) {
        monoWidth = 1 + *std::max_element(fontWidthsPager.cbegin(),
                                          fontWidthsPager.cend());
    }
    fonts[font] = FontMetaData{
        textureName,
        *glyphWidths,
        textureSize,
        glyphOffset,
        monoWidth
    };
}

void TextRenderer::renderText(const TextRenderer::TextInfo& ti,
                              bool forceColour) {
    if (ti.text.empty() || ti.text[0] == '*')
        return;

    renderer->getRenderer()->pushDebugGroup("Text");
    renderer->getRenderer()->useProgram(textShader.get());

    glm::vec2 coord(0.f, 0.f);
    glm::vec2 alignment = ti.screenPosition;
    // We should track real size not just chars.
    auto lineLength = 0;

    glm::vec2 ss(ti.size);

    glm::vec3 colour = glm::vec3(ti.baseColour) * (1 / 255.f);
    glm::vec4 colourBG = glm::vec4(ti.backgroundColour) * (1 / 255.f);
    std::vector<TextVertex> geo;

    float maxWidth = 0.f;
    float maxHeight = ss.y;

    auto text = ti.text;

    const auto &fontMetaData = fonts[ti.font];

    for (size_t i = 0; i < text.length(); ++i) {
        char16_t c = text[i];

        // Handle any markup changes.
        if (c == '~' && text.length() > i + 2) {
            switch (text[i + 1]) {
                case 'b':  // Blue
                    text.erase(text.begin() + i, text.begin() + i + 3);
                    colour = glm::vec3(glm::u8vec3(128, 167, 243)) * (1 / 255.f);
                    break;
                case 'g':  // Green
                    text.erase(text.begin() + i, text.begin() + i + 3);
                    colour = glm::vec3(glm::u8vec3(95, 160, 106)) * (1 / 255.f);
                    break;
                case 'h':  // White
                    text.erase(text.begin() + i, text.begin() + i + 3);
                    colour = glm::vec3(glm::u8vec3(225, 225, 225)) * (1 / 255.f);
                    break;
                case 'k': {  // Key
                    text.erase(text.begin() + i, text.begin() + i + 3);
                    // Extract the key name from the /next/ markup
                    auto keyend = text.find('~', i + 1);
                    auto keyname = text.substr(i + 1, keyend - i - 1);
                    // Since we don't have a key map yet, just print out the
                    // name
                    text.erase(text.begin() + i, text.begin() + keyend);
                    text.insert(i, keyname);
                    break;
                }
                case 'l':  // Black
                    text.erase(text.begin() + i, text.begin() + i + 3);
                    colour = glm::vec3(0.f);
                    break;
                case 'p':  // Purple
                    text.erase(text.begin() + i, text.begin() + i + 3);
                    colour = glm::vec3(glm::u8vec3(168, 110, 252)) * (1 / 255.f);
                    break;
                case 'r':  // Red
                    text.erase(text.begin() + i, text.begin() + i + 3);
                    colour = glm::vec3(glm::u8vec3(113, 43, 73)) * (1 / 255.f);
                    break;
                case 'w':  // Gray
                    text.erase(text.begin() + i, text.begin() + i + 3);
                    colour = glm::vec3(glm::u8vec3(175, 175, 175)) * (1 / 255.f);
                    break;
                case 'y':  // Yellow
                    text.erase(text.begin() + i, text.begin() + i + 3);
                    colour = glm::vec3(glm::u8vec3(210, 196, 106)) * (1 / 255.f);
                    break;
            }

            c = text[i];
        }

        if (forceColour) {
            colour = glm::vec3(ti.baseColour) * (1 / 255.f);
        }

        // Handle special chars.
        if (c == '\n') {
            coord.x = 0.f;
            coord.y += ss.y;
            maxHeight = coord.y + ss.y;
            lineLength = 0;
            continue;
        }

        auto glyph = charToIndex(c);
        if (glyph >= fontMetaData.glyphWidths.size()) {
            continue;
        }

        // If we're not at the start of the column, check if the current word
        // will need to be wrapped
        if (ti.wrapX > 0 && coord.x > 0.f && !std::isspace(c)) {
            auto wend = std::find_if(std::begin(text) + i, std::end(text),
                                     [](char x) { return std::isspace(x); });
            if (wend != std::end(text)) {
                auto word = std::distance(std::begin(text) + i, wend);
                if (lineLength + word >= ti.wrapX) {
                    coord.x = 0;
                    coord.y += ss.y;
                    maxHeight = coord.y + ss.y;
                    lineLength = 0;
                }
            }
        }

        auto tex = indexToTexCoord(glyph, fontMetaData.textureSize, fontMetaData.glyphOffset);
        ss.x = ti.size * static_cast<float>(fontMetaData.glyphOffset.x) / fontMetaData.glyphOffset.y;
        lineLength++;

        glm::vec2 p = coord;
        float factor = ti.size / static_cast<float>(fontMetaData.glyphOffset.y);
        float glyphWidth = factor * static_cast<float>(fontMetaData.glyphWidths[glyph]);
        if (fontMetaData.monoWidth != 0) {
            float monoWidth = factor * fontMetaData.monoWidth;
            p.x += static_cast<float>(monoWidth - glyphWidth) / 2;
            coord.x += monoWidth;
        } else {
            coord.x += glyphWidth;
        }
        maxWidth = std::max(coord.x, maxWidth);

        geo.emplace_back(glm::vec2{p.x, p.y + ss.y}, glm::vec2{tex.x, tex.w}, colour);
        geo.emplace_back(glm::vec2{p.x + ss.x, p.y + ss.y}, glm::vec2{tex.z, tex.w}, colour);
        geo.emplace_back(glm::vec2{p.x, p.y}, glm::vec2{tex.x, tex.y}, colour);

        geo.emplace_back(glm::vec2{p.x + ss.x, p.y}, glm::vec2{tex.z, tex.y}, colour);
        geo.emplace_back(glm::vec2{p.x, p.y}, glm::vec2{tex.x, tex.y}, colour);
        geo.emplace_back(glm::vec2{p.x + ss.x, p.y + ss.y}, glm::vec2{tex.z, tex.w}, colour);
    }

    if (ti.align == TextInfo::TextAlignment::Right) {
        alignment.x -= maxWidth;
    } else if (ti.align == TextInfo::TextAlignment::Center) {
        alignment.x -= (maxWidth / 2.f);
    }

    alignment.y -= ti.size * 0.2f;

    // If we need to, draw the background.
    if (colourBG.a > 0.f) {
        renderer->drawColour(
            colourBG, glm::vec4(ti.screenPosition - (ss / 3.f),
                                glm::vec2(maxWidth, maxHeight) + (ss / 2.f)));
    }

    renderer->getRenderer()->setUniform(
        textShader.get(), "proj", renderer->getRenderer()->get2DProjection());
    renderer->getRenderer()->setUniformTexture(textShader.get(), "fontTexture", 0);
    renderer->getRenderer()->setUniform(textShader.get(), "alignment", alignment);

    gb.uploadVertices(geo);
    db.addGeometry(&gb);
    db.setFaceType(GL_TRIANGLES);

    Renderer::DrawParameters dp;
    dp.start = 0;
    dp.blendMode = BlendMode::BLEND_ALPHA;
    dp.count = gb.getCount();
    auto ftexture = renderer->getData()->findSlotTexture("fonts", fontMetaData.textureName);
    dp.textures = {ftexture->getName()};
    dp.depthMode = DepthMode::OFF;

    renderer->getRenderer()->drawArrays(glm::mat4(1.0f), &db, dp);

    renderer->getRenderer()->popDebugGroup();
}
