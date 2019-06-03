#include <viewer/RWViewerWindows.hpp>

#include <engine/GameWorld.hpp>
#include <loaders/LoaderGXT.hpp>
#include <objects/InstanceObject.hpp>
#include <platform/FileHandle.hpp>

#include <imgui.h>

#include <algorithm>

namespace {
std::array<int, 3> kFonts{{FONT_PAGER, FONT_PRICEDOWN, FONT_ARIAL}};
std::array<const char*, 3> kFontNames{{"Pager", "Pricedown", "Arial"}};

std::vector<std::string> getFontTextureNames(GameData& data) {
    const auto& gameDataPath = rwfs::path(data.getDataPath());
    rwfs::path textPath;
    for (const rwfs::path& p : rwfs::directory_iterator(gameDataPath)) {
        if (!rwfs::is_directory(p)) {
            continue;
        }
        std::string filename = p.filename().string();
        std::transform(filename.begin(), filename.end(), filename.begin(),
                       ::tolower);
        if (filename == "text") {
            textPath = p;
            break;
        }
    }
    if (!textPath.string().length()) {
        throw std::runtime_error("text directory not found in gamedata path");
    }
    std::vector<std::string> names;
    for (const rwfs::path& p : rwfs::directory_iterator(textPath)) {
        auto langName = p.filename().string();
        std::transform(langName.begin(), langName.end(), langName.begin(),
                       ::tolower);
        names.emplace_back(std::move(langName));
    }
    return names;
}
}  // namespace

TextViewer::TextViewer(GameData& data)
    : data_(data), languages_(getFontTextureNames(data)) {
    LoaderGXT loader;
    for (const auto& textName : languages_) {
        GameTexts texts;
        auto handle = data.index.openFile(textName);
        loader.load(texts, handle);
        const auto& language = textName;
        const auto& stringTable = texts.getStringTable();
        for (const auto& tableItem : stringTable) {
            texts_[tableItem.first][language] = {tableItem.second, ""};
        }
    }

    cacheStrings();
}

void TextViewer::cacheStrings() {
    for (auto& text : texts_) {
        for (const auto& l : languages_) {
            auto it = text.second.find(l);
            if (it != text.second.end()) {
                it->second.second = GameStringUtil::toString(
                    it->second.first, kFonts[currentFont_]);
            }
        }
    }
}

void TextViewer::draw(GameRenderer& r) {
    ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
    if (ImGui::Combo("##font", &currentFont_, kFontNames.data(),
                     kFontNames.size()))
        cacheStrings();
    ImGui::PopItemWidth();

    ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.1f);
    ImGui::SameLine();
    ImGui::InputFloat("##font_size", &currentFontSize_);
    ImGui::PopItemWidth();

    ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);
    ImGui::SameLine();
    if (ImGui::InputText("##font_text", previewStr_, 512)) {
        previewText_ =
            GameStringUtil::fromString(previewStr_, kFonts[currentFont_]);
    }
    ImGui::PopItemWidth();

    ImGui::SameLine();
    ImGui::Checkbox("Draw Preview", &drawPreview_);

    ImGui::Columns(static_cast<int>(1 + languages_.size()));
    ImGui::Text("Code");
    ImGui::NextColumn();
    for (const auto& l : languages_) {
        ImGui::Text("%s", l.c_str());
        ImGui::NextColumn();
    }
    ImGui::Separator();

    for (const auto& [code, text] : texts_) {
        ImGui::Text("%s", code.c_str());
        ImGui::NextColumn();

        for (const auto& l : languages_) {
            auto it = text.find(l);
            if (it != text.end()) {
                if (ImGui::Selectable(it->second.second.c_str(), false,
                                      ImGuiSelectableFlags_AllowDoubleClick)) {
                    if (ImGui::IsMouseDoubleClicked(0)) {
                        strncpy(previewStr_, it->second.second.c_str(), 512);
                        previewText_ = it->second.first;
                    }
                }
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("%s", it->second.second.c_str());
            }
            ImGui::NextColumn();
        }
    }

    if (!drawPreview_) return;

    {
        TextRenderer::TextInfo ti;
        ti.font = kFonts[currentFont_];
        ti.size = currentFontSize_;
        ti.baseColour = glm::u8vec3(255);
        ti.backgroundColour = glm::u8vec4(0, 0, 0, 0);
        ti.align = TextRenderer::TextInfo::TextAlignment::Left;
        ti.wrapX = 0;

        ti.text = previewText_;
        ti.screenPosition = glm::vec2(10, 50);
        r.text.renderText(ti, false);
    }

    {
        TextRenderer::TextInfo ti;
        ti.font = kFonts[currentFont_];
        ti.size = currentFontSize_;
        ti.baseColour = glm::u8vec3(255);
        ti.backgroundColour = glm::u8vec4(0, 0, 0, 0);
        ti.align = TextRenderer::TextInfo::TextAlignment::Left;
        ti.wrapX = 0;

        const auto ROW_STRIDE = currentFontSize_ * 1.2f;
        const auto COL_STRIDE = currentFontSize_ * 1.2f;

        for (GameStringChar c = 0x20; c < 0xb2; ++c) {
            unsigned column = c % 0x10;
            unsigned row = (c / 0x10) - 2 + 3; /* +3 to offset first line*/
            ti.text = c;
            ti.screenPosition =
                glm::vec2(10 + (column * COL_STRIDE), 50 + (row * ROW_STRIDE));
            r.text.renderText(ti, false);
        }
    }
}

IPLViewer::IPLViewer(Logger& log, GameData& data)
    : data_(data), world_(std::make_unique<GameWorld>(&log, &data)) {
    std::transform(begin(data_.iplLocations), end(data_.iplLocations),
                   std::back_inserter(ipls_), [](const auto& a) -> IPLFileData {
                       return {false, a.first, a.second, {}, {}};
                   });
    world_->state = &state_;
}

void IPLViewer::draw(GameRenderer& r) {
    for (auto& ipl : ipls_) {
        if (ImGui::Checkbox(ipl.name.c_str(), &ipl.enabled)) {
            if (ipl.enabled)
                showIPL(ipl);
            else
                hideIPL(ipl);
        }
    }
}

void IPLViewer::showIPL(IPLViewer::IPLFileData& ipl) {
    if (ipl.loader.m_instances.empty()) {
        ipl.loader.load(ipl.path);
    }

    for (const auto& inst : ipl.loader.m_instances) {
        auto instance = world_->createInstance(inst.id, inst.pos, inst.rot);
        if (!instance) {
            world_->logger->error("World", "No object data for instance " +
                                               std::to_string(inst.id) +
                                               " in " + ipl.name);
        } else {
            ipl.objects.push_back(instance);
        }
    }
}

void IPLViewer::hideIPL(IPLViewer::IPLFileData& ipl) {
    for (auto& object : ipl.objects) {
        world_->destroyObject(object);
    }
    ipl.objects.clear();
}
