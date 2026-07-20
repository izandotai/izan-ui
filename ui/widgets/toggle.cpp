#include "ui/widgets/toggle.hpp"

#include <imgui.h>

#include "ui/render/sdf_rect.hpp"
#include "ui/widgets/design.hpp"

namespace izan::ui {

bool kit_toggle(const char* id, bool* value)
{
    const float em = ImGui::GetFontSize();
    const float height = em * 1.05f;
    const float width = height * 1.75f;
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const bool clicked = ImGui::InvisibleButton(id, ImVec2(width, height));
    if (clicked)
        *value = !*value;
    if (ImGui::IsItemHovered())
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

    // The knob eases across; its resting place is remembered per id so
    // the motion survives immediate mode.
    ImGuiStorage* storage = ImGui::GetStateStorage();
    const ImGuiID key = ImGui::GetID(id);
    float t = storage->GetFloat(key, *value ? 1.0f : 0.0f);
    const float target = *value ? 1.0f : 0.0f;
    const float speed = ImGui::GetIO().DeltaTime * 10.0f;
    t = t + (target - t) * (speed > 1.0f ? 1.0f : speed) * 2.0f;
    if (t < 0.0f)
        t = 0.0f;
    if (t > 1.0f)
        t = 1.0f;
    storage->SetFloat(key, t);

    ImDrawList* draw = ImGui::GetWindowDrawList();
    const ImVec4 off = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
    ImVec4 on = kit_accent();
    on.w = 1.0f;
    const ImU32 track = ImGui::GetColorU32(kit_blend(off, on, t));
    {
        render::SdfRect capsule;
        capsule.min = pos;
        capsule.max = ImVec2(pos.x + width, pos.y + height);
        capsule.radius[0] = capsule.radius[1] = capsule.radius[2]
            = capsule.radius[3] = height * 0.5f;
        capsule.fill = track;
        render::sdf_rect(draw, capsule);
    }
    const float pad = height * 0.12f;
    const float radius = height * 0.5f - pad;
    const float x = pos.x + height * 0.5f + t * (width - height);
    draw->AddCircleFilled(
        ImVec2(x, pos.y + height * 0.5f), radius, IM_COL32(255, 255, 255, 245));
    return clicked;
}

}
