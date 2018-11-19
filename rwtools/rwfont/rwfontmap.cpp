#include <fonts/FontMapGta3.hpp>
#include <fonts/Unicode.hpp>
#include <rw/filesystem.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <boost/program_options.hpp>

#include <QImage>
#include <QImageWriter>

#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

const char *ft_error_to_string(int error) {
    switch (error) {
#define FT_NOERRORDEF_(_DC, CODE, STR) case CODE: return STR;
#define FT_ERRORDEF_(_DC, CODE, STR) case CODE: return STR;
#include FT_ERROR_DEFINITIONS_H
#undef FT_ERRORDEF_
#undef FT_NOERRORDEF_
    }
    return "<unknown>";
}

[[noreturn]]
void ft_error(int code)
{
    std::cerr << ft_error_to_string(code) << "\n";
    exit(1);
}

class FontTextureBuffer {
public:
    FontTextureBuffer(unsigned fontsize, unsigned width, unsigned height, float height_width_ratio)
        : m_texture{static_cast<int>(width), static_cast<int>(height), QImage::Format_ARGB32} {
        m_aspectratio = height_width_ratio;
        m_fontsize = fontsize;
        m_glyph_width = width / 16;
        m_glyph_height = height_width_ratio * m_glyph_width;

        int error = FT_Init_FreeType(&m_library);
        if (error != 0) {
            ft_error(error);
        }
    }

    ~FontTextureBuffer() {
        for (auto face : m_faces) {
            int error = FT_Done_Face(face);
            if (error != 0) {
                ft_error(error);
            }
        }
        int error = FT_Done_FreeType(m_library);
        if (error != 0) {
            ft_error(error);
        }
    }

    void clear() {
        m_texture.fill(QColor{0, 0, 0, 0});
        m_advances.clear();
    }

    void add_face(const char *path) {
        m_faces.emplace_back();
        FT_Face &face = m_faces.back();
        int error = FT_New_Face(m_library, path, 0, &face);
        if (error != 0) {
            ft_error(error);
        }
        error = FT_Select_Charmap(face, FT_ENCODING_UNICODE);
        if (error != 0) {
            ft_error(error);
        }
        error = FT_Set_Pixel_Sizes(face, 0, m_fontsize);
        if (error != 0) {
            ft_error(error);
        }
    }

    void draw_glyph_mono(int index, FT_GlyphSlot glyph) {
        int x = index % 16;
        int y = index / 16 - 2;
        QPoint topleft = {x * static_cast<int>(m_glyph_width), y * static_cast<int>(m_glyph_height)};
        QPoint baselineleft = topleft + QPoint{0, static_cast<int>(m_glyph_width)};

        if (baselineleft.x() + glyph->metrics.horiBearingX / 64 + glyph->metrics.width / 64 >= topleft.x() + m_glyph_width) {
            std::cerr << "index " << index << " crosses right border\n";
        }
        if (baselineleft.y() - glyph->metrics.horiBearingY / 64 < topleft.y()) {
            std::cerr << "index " << index << " crosses top border\n";
        }
        if (baselineleft.y() - glyph->metrics.horiBearingY / 64 + glyph->metrics.height / 64 >= topleft.y() + m_glyph_height) {
            std::cerr << "index " << index << " crosses bottom border\n";
        }

        for (unsigned row = 0; row < glyph->bitmap.rows; ++row) {
            const unsigned char *buffer = glyph->bitmap.buffer + row * glyph->bitmap.pitch;
            for (unsigned i = 0; i < glyph->bitmap.width; ++i) {
                bool pixel = ((buffer[i / 8] << (i % 8)) & 0x80) != 0;
                QPoint point = baselineleft + QPoint{static_cast<int>(i), static_cast<int>(row - (glyph->metrics.horiBearingY / 64))};
                QColor color{255, 255, 255, pixel ? 255 : 0};
                m_texture.setPixelColor(point, color);
            }
        }
    }

    void draw_glyph_normal(int index, FT_GlyphSlot glyph) {
        int x = index % 16;
        int y = index / 16 - 2;
        QPoint topleft = {x * static_cast<int>(m_glyph_width), y * static_cast<int>(m_glyph_height)};
        QPoint baselineleft = topleft + QPoint{0, 4 * static_cast<int>(m_glyph_height) / 5};

        if (baselineleft.x() + glyph->metrics.horiBearingX / 64 + glyph->metrics.width / 64 >= topleft.x() + m_glyph_width) {
            std::cerr << "index " << index << " crosses right border\n";
        }
        if (baselineleft.y() - glyph->metrics.horiBearingY / 64 < topleft.y()) {
            std::cerr << "index " << index << " crosses top border\n";
        }
        if (baselineleft.y() - glyph->metrics.horiBearingY / 64 + glyph->metrics.height / 64 >= topleft.y() + m_glyph_height) {
            std::cerr << "index " << index << " crosses bottom border\n";
        }

        for (unsigned row = 0; row < glyph->bitmap.rows; ++row) {
            const unsigned char *buffer = glyph->bitmap.buffer + row * glyph->bitmap.pitch;
            for (unsigned i = 0; i < glyph->bitmap.width; ++i) {
                QPoint point = baselineleft + QPoint{static_cast<int>(i), static_cast<int>(row - (glyph->metrics.horiBearingY / 64))};
                QColor color{255, 255, 255, buffer[i]};
                m_texture.setPixelColor(point, color);
            }
        }
    }

    enum RenderMode {
        MONO,
        NORMAL
    };

    void create_font_map(const FontMap &fontmap, RenderMode mode) {
        GameStringChar next = 0x20;
        FT_Render_Mode render_mode;
        switch (mode) {
        case RenderMode::MONO:
            render_mode = FT_RENDER_MODE_MONO;
            break;
        default:
        case RenderMode::NORMAL:
            render_mode = FT_RENDER_MODE_NORMAL;
            break;
        }
        for (auto it = fontmap.to_unicode_begin(); it != fontmap.to_unicode_end(); ++it) {
            while (it->first != next) {
                m_advances.push_back(0);
                ++next;
            }
            for (auto face : m_faces) {
                auto glyph_index = FT_Get_Char_Index(face, it->second);
                if (glyph_index == 0) {
                    continue;
                }
                int error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
                if (error != 0) {
                    ft_error(error);
                }
                error = FT_Render_Glyph(face->glyph, render_mode);
                if (error != 0) {
                    ft_error(error);
                }

                switch (mode) {
                case RenderMode::MONO:
                    draw_glyph_mono(it->first, face->glyph);
                    break;
                case RenderMode::NORMAL:
                    draw_glyph_normal(it->first, face->glyph);
                    break;
                }

                int advance = face->glyph->metrics.horiAdvance / 64;// - face->glyph->metrics.horiBearingX / 64;
                if (advance < 0 || advance > 255) {
                    std::cerr << "advance out of range\n";
                }
                m_advances.push_back(advance);
                ++next;
                break;
            }
            if (it->first + 1 != next) {
                std::cerr << "unknown character.\n"; exit(1);

            }
        }
    }

    void write(const rwfs::path &bitmap_path, const rwfs::path &advance_path) {
        QImageWriter writer(QString::fromStdString(bitmap_path.string()));
        writer.write(m_texture);
        std::ofstream ofs(advance_path.string(), std::ios_base::out);
        ofs << m_aspectratio << '\n';
        for (auto adv : m_advances) {
            ofs << int(adv) << '\n';
        }
//        ofs.write(reinterpret_cast<const char *>(m_advances.data()), m_advances.size());
    }

private:
    QImage m_texture;

    float m_aspectratio;
    unsigned m_fontsize;
    unsigned m_glyph_width;
    unsigned m_glyph_height;
    FT_Library m_library = nullptr;

    std::vector<FT_Face> m_faces;

    std::vector<std::uint8_t> m_advances;
};

int main(int argc, const char *argv[])
{
    namespace po = boost::program_options;
    po::options_description desc("Options");
    desc.add_options()
        ("help", "Show this help message")
        ("fontsize,s",  po::value<unsigned>()->value_name("FONTSIZE")->required(), "Fontsize")
        ("width,w",  po::value<unsigned>()->value_name("WIDTH")->required(), "Width of the texture")
        ("height,h",  po::value<unsigned>()->value_name("HEIGHT")->required(), "Height of the texture")
        ("ratio,r",  po::value<float>()->value_name("ASPECTRATIO"), "Aspect ratio")
        ("map,m",  po::value<unsigned>()->value_name("MAP")->required(), "Font map to use")
        ("font,f", po::value<std::vector<std::string>>()->value_name("PATH")->required(), "Path to fonts")
        ("texture,t", po::value<rwfs::path>()->value_name("PATH")->required(), "Output texture")
        ("advance,a", po::value<rwfs::path>()->value_name("PATH")->required(), "Output advances")
    ;

    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        if (vm.count("help") != 0u) {
            std::cout << desc;
            return EXIT_SUCCESS;
        }
        po::notify(vm);
    } catch (po::error &ex) {
        std::cerr << "Error parsing arguments: " << ex.what() << std::endl;
        std::cerr << desc;
        return EXIT_FAILURE;
    }

    const auto fontmap_index = vm["map"].as<unsigned>();
    const auto width = vm["width"].as<unsigned>();
    const auto height = vm["height"].as<unsigned>();
    const auto fontsize = vm["fontsize"].as<unsigned>();

    float aspect;
    if (vm.count("aspect") != 0u) {
        aspect = vm["aspect"].as<float>();
    } else {
        if (fontmap_index == 0) {
            aspect = 1.0f;
        } else {
            aspect = 1.25f;
        }
    }

    if (fontmap_index >= fontmaps_gta3_font.size()) {
        std::cerr << "Illegal map: range: [0, " << fontmaps_gta3_font.size() << ")\n";
        std::cerr << desc;
        return EXIT_FAILURE;
    }

    FontTextureBuffer texBuffer{fontsize, width, height, aspect};

    for (const auto &fontpath : vm["font"].as<std::vector<std::string>>()) {
        texBuffer.add_face(fontpath.c_str());
    }

    texBuffer.create_font_map(fontmaps_gta3_font[fontmap_index], FontTextureBuffer::RenderMode::NORMAL);

    texBuffer.write(vm["texture"].as<rwfs::path>(), vm["advance"].as<rwfs::path>());
    return 0;
}
