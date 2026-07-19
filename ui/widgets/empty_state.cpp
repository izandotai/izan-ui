#include "ui/widgets/empty_state.hpp"

#include <imgui.h>

#include "ui/widgets/label.hpp"

namespace izan::ui {

void kit_empty_state(const char* glyph, const char* caption)
{
    const float em = ImGui::GetFontSize();
    const float width = ImGui::GetWindowSize().x;
    auto centered = [&](float w) {
        const float x = (width - w) * 0.5f;
        ImGui::SetCursorPosX(x > 0.0f ? x : 0.0f);
    };

    ImGui::Dummy(ImVec2(0.0f, ImGui::GetContentRegionAvail().y * 0.28f));
    ImGui::PushFont(nullptr, em * 2.4f);
    centered(ImGui::CalcTextSize(glyph).x);
    ImGui::TextDisabled("%s", glyph);
    ImGui::PopFont();
    kit_vspace(0.4f);
    ImGui::PushFont(nullptr, kit_caption_size());
    centered(ImGui::CalcTextSize(caption).x);
    ImGui::TextDisabled("%s", caption);
    ImGui::PopFont();
}

}
