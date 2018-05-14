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

int charToIndex(uint16_t g) {
    // Correct for the default font maps
    /// @todo confirm for JA / RU font maps
    return g - 32;
}

glm::vec4 indexToCoord(int font, int index) {
    float x = int(index % 16);
    float y = int(index / 16) + 0.01f;
    float fontHeight = ((font == 0) ? 16.f : 13.f);
    glm::vec2 gsize(1.f / 16.f, 1.f / fontHeight);
    return glm::vec4(x, y, x + 1, y + 0.98f) * glm::vec4(gsize, gsize);
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

TextRenderer::TextInfo::TextInfo()
    : font(0), size(1.f), baseColour({1.f, 1.f, 1.f}), align(Left), wrapX(0) {
}

TextRenderer::TextRenderer(GameRenderer* renderer) : renderer(renderer) {
    textShader = renderer->getRenderer()->createShader(TextVertexShader,
                                                       TextFragmentShader);

    std::fill(glyphData.begin(), glyphData.end(), GlyphInfo{.9f});

    glyphData[charToIndex(' ')].widthFrac = 0.4f;
    glyphData[charToIndex('-')].widthFrac = 0.5f;
    glyphData[charToIndex('\'')].widthFrac = 0.5f;
    glyphData[charToIndex('(')].widthFrac = 0.45f;
    glyphData[charToIndex(')')].widthFrac = 0.45f;
    glyphData[charToIndex(':')].widthFrac = 0.65f;
    glyphData[charToIndex('$')].widthFrac = 0.65f;

    for (char g = '0'; g <= '9'; ++g) {
        glyphData[charToIndex(g)].widthFrac = 0.65f;
    }

    // Assumes contigious a-z character encoding
    for (char g = 0; g <= ('z' - 'a'); g++) {
        switch (('a' + g)) {
            case 'i':
                glyphData[charToIndex('a' + g)].widthFrac = 0.4f;
                glyphData[charToIndex('A' + g)].widthFrac = 0.4f;
                break;
            case 'l':
                glyphData[charToIndex('a' + g)].widthFrac = 0.5f;
                glyphData[charToIndex('A' + g)].widthFrac = 0.5f;
                break;
            case 'm':
                glyphData[charToIndex('a' + g)].widthFrac = 1.0f;
                glyphData[charToIndex('A' + g)].widthFrac = 1.0f;
                break;
            case 'w':
                glyphData[charToIndex('a' + g)].widthFrac = 1.0f;
                glyphData[charToIndex('A' + g)].widthFrac = 1.0f;
                break;
            default:
                glyphData[charToIndex('a' + g)].widthFrac = 0.7f;
                glyphData[charToIndex('A' + g)].widthFrac = 0.7f;
                break;
        }
    }
}

TextRenderer::~TextRenderer() = default;

void TextRenderer::setFontTexture(int index, const std::string& texture) {
    if (index < GAME_FONTS) {
        fonts[index] = texture;
    }
}

void TextRenderer::renderText(const TextRenderer::TextInfo& ti,
                              bool forceColour) {
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
    float maxHeight = 0.f;

    auto text = ti.text;

    for (size_t i = 0; i < text.length(); ++i) {
        char16_t c = text[i];

        // Handle any markup changes.
        if (c == '~' && text.length() > i + 1) {
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

        int glyph = charToIndex(c);
        if (glyph >= GAME_GLYPHS) {
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

        auto& data = glyphData[glyph];
        auto tex = indexToCoord(ti.font, glyph);

        ss.x = ti.size * data.widthFrac;
        tex.z = tex.x + (tex.z - tex.x) * data.widthFrac;

        // Handle special chars.
        if (c == '\n') {
            coord.x = 0.f;
            coord.y += ss.y;
            maxHeight = coord.y + ss.y;
            lineLength = 0;
            continue;
        }
        lineLength++;

        glm::vec2 p = coord;
        coord.x += ss.x;
        maxWidth = std::max(coord.x, maxWidth);

        geo.emplace_back(glm::vec2{p.x, p.y + ss.y}, glm::vec2{tex.x, tex.w}, colour);
        geo.emplace_back(glm::vec2{p.x + ss.x, p.y + ss.y}, glm::vec2{tex.z, tex.w}, colour);
        geo.emplace_back(glm::vec2{p.x, p.y}, glm::vec2{tex.x, tex.y}, colour);

        geo.emplace_back(glm::vec2{p.x + ss.x, p.y}, glm::vec2{tex.z, tex.y}, colour);
        geo.emplace_back(glm::vec2{p.x, p.y}, glm::vec2{tex.x, tex.y}, colour);
        geo.emplace_back(glm::vec2{p.x + ss.x, p.y + ss.y}, glm::vec2{tex.z, tex.w}, colour);
    }

    if (ti.align == TextInfo::Right) {
        alignment.x -= maxWidth;
    } else if (ti.align == TextInfo::Center) {
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
    auto ftexture = renderer->getData()->findSlotTexture("fonts", fonts[ti.font]);
    dp.textures = {ftexture->getName()};
    dp.depthWrite = false;

    renderer->getRenderer()->drawArrays(glm::mat4(1.0f), &db, dp);

    renderer->getRenderer()->popDebugGroup();
}
