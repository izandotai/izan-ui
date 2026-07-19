#include "ui/widgets/result_mark.hpp"

#include <imgui.h>

#include "ui/widgets/design.hpp"

namespace izan::ui {

void kit_result_mark(bool ok, float size_em)
{
    const float d = ImGui::GetFontSize() * size_em;
    const float slack = ImGui::GetContentRegionAvail().x - d;
    if (slack > 0.0f)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + slack * 0.5f);
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::Dummy(ImVec2(d, d));

    ImDrawList* draw = ImGui::GetWindowDrawList();
    const ImVec2 center(pos.x + d * 0.5f, pos.y + d * 0.5f);
    const ImVec4 tone = ok ? kit_accent() : kit_danger();
    draw->AddCircleFilled(center, d * 0.5f, ImGui::GetColorU32(tone), 0);

    const ImU32 stroke = IM_COL32(255, 255, 255, 235);
    const float t = d * 0.075f;
    auto at = [&](float x, float y) {
        return ImVec2(pos.x + d * x, pos.y + d * y);
    };
    if (ok) {
        const ImVec2 tick[3]
            = { at(0.28f, 0.53f), at(0.44f, 0.68f), at(0.72f, 0.35f) };
        draw->AddPolyline(tick, 3, stroke, ImDrawFlags_None, t);
    } else {
        draw->AddLine(at(0.34f, 0.34f), at(0.66f, 0.66f), stroke, t);
        draw->AddLine(at(0.66f, 0.34f), at(0.34f, 0.66f), stroke, t);
    }
}

}
