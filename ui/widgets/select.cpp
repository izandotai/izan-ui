#include "ui/widgets/select.hpp"

#include <algorithm>

#include <imgui.h>
#include <imgui_internal.h>

#include "ui/widgets/design.hpp"
#include "ui/widgets/text_field.hpp"

namespace izan::ui {

namespace {

    void paint_chevron(ImDrawList* draw, const ImRect& box, bool emphasized)
    {
        const float em = ImGui::GetFontSize();
        const float arm = em * 0.24f;
        const ImVec2 center(box.Max.x - em * design().field_pad_x - arm,
            (box.Min.y + box.Max.y) * 0.5f);
        const ImU32 color = ImGui::GetColorU32(
            emphasized ? ImGuiCol_Text : ImGuiCol_TextDisabled);
        const float thickness = std::max(1.5f, em * 0.085f);
        draw->AddLine(ImVec2(center.x - arm, center.y - arm * 0.45f),
            ImVec2(center.x, center.y + arm * 0.45f), color, thickness);
        draw->AddLine(ImVec2(center.x, center.y + arm * 0.45f),
            ImVec2(center.x + arm, center.y - arm * 0.45f), color, thickness);
    }

    void paint_single_focus_ring(ImDrawList* draw, const ImRect& box)
    {
        ImVec4 ring = kit_accent();
        ring.w = 0.82f;
        kit_round_border(draw, box.Min, box.Max,
            ImGui::GetFontSize() * design().field_radius, ring,
            design().border_px);
    }

}

bool kit_select_begin(const char* id, const char* preview, float width)
{
    ImGuiContext& g = *ImGui::GetCurrentContext();
    ImGuiWindow* window = ImGui::GetCurrentWindow();

    // Match BeginCombo's NextWindowData contract: size constraints belong to
    // the popup, not the framed item that opens it.
    const ImGuiNextWindowDataFlags next_window_flags
        = g.NextWindowData.HasFlags;
    g.NextWindowData.ClearFlags();
    if (window->SkipItems)
        return false;

    const float em = ImGui::GetFontSize();
    const DesignLanguage& dl = design();
    const ImVec2 pad(em * dl.field_pad_x, em * dl.field_pad_y);
    const char* label_end = ImGui::FindRenderedTextEnd(id);
    const ImVec2 label_size = ImGui::CalcTextSize(id, label_end, false);
    const float w = width > 0.0f ? width : ImGui::CalcItemWidth();
    const ImRect box(window->DC.CursorPos,
        ImVec2(window->DC.CursorPos.x + w,
            window->DC.CursorPos.y + em + pad.y * 2.0f));
    const ImRect total(box.Min,
        ImVec2(box.Max.x
                + (label_size.x > 0.0f
                        ? ImGui::GetStyle().ItemInnerSpacing.x + label_size.x
                        : 0.0f),
            box.Max.y));
    const ImGuiID item_id = window->GetID(id);
    ImGui::ItemSize(total, pad.y);
    if (!ImGui::ItemAdd(total, item_id, &box))
        return false;

    bool hovered = false;
    bool held = false;
    const bool pressed = ImGui::ButtonBehavior(box, item_id, &hovered, &held);
    const ImGuiID popup_id = ImHashStr("##KitSelectPopup", 0, item_id);
    bool popup_open = ImGui::IsPopupOpen(popup_id, ImGuiPopupFlags_None);
    if (pressed && !popup_open) {
        ImGui::OpenPopupEx(popup_id, ImGuiPopupFlags_None);
        popup_open = true;
    }
    if (hovered)
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

    // One uninterrupted input well: no native arrow button, split fill or
    // polygon frame. The preview uses the same inset and baseline as text
    // fields; the chevron lives inside a quiet trailing zone.
    const bool keyboard_focus = g.NavId == item_id && g.NavCursorVisible;
    kit_field_frame(box.Min, box.GetSize());
    if (popup_open || held || keyboard_focus)
        paint_single_focus_ring(window->DrawList, box);
    const float arrow_zone = em * (dl.field_pad_x + 0.9f);
    if (preview != nullptr)
        ImGui::RenderTextClipped(ImVec2(box.Min.x + pad.x, box.Min.y + pad.y),
            ImVec2(box.Max.x - arrow_zone, box.Max.y - pad.y), preview, nullptr,
            nullptr, ImVec2(0, 0));
    paint_chevron(window->DrawList, box, hovered || popup_open);
    if (label_size.x > 0.0f)
        ImGui::RenderText(
            ImVec2(box.Max.x + ImGui::GetStyle().ItemInnerSpacing.x,
                box.Min.y + pad.y),
            id, label_end, false);

    if (!popup_open)
        return false;

    g.NextWindowData.HasFlags = next_window_flags;
    if (!ImGui::BeginComboPopup(popup_id, box, ImGuiComboFlags_HeightRegular))
        return false;
    return true;
}

bool kit_select_item(const char* label, bool selected)
{
    const bool clicked = ImGui::Selectable(label, selected);
    if (selected)
        ImGui::SetItemDefaultFocus();
    return clicked;
}

void kit_select_end()
{
    ImGui::EndCombo();
}

}
