#include "ui/widgets/menu.hpp"

#include <imgui.h>
#include <imgui_internal.h>

#include "ui/widgets/avatar.hpp"
#include "ui/widgets/design.hpp"
#include "ui/widgets/label.hpp"

namespace izan::ui {

namespace {

    // The shell's dropdown numbers, mirrored exactly — one rhythm for
    // every menu in the app (chrome_widgets pushes the same values on
    // the menu bar's own popups).
    // Same recipe as the dialog shell: imgui paints no popup of its
    // own; the panel below is analytic, corners computed.
    ImVec4 g_menu_bg;

    void push_menu_style()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 14.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(16.0f, 11.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(12.0f, 12.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 0.0f);
        // Captured, never pushed — see the dialog shell's note on the
        // stolen transparent push; NoBackground on the popup flags is
        // theft-proof.
        g_menu_bg = ImGui::GetStyleColorVec4(ImGuiCol_PopupBg);
        g_menu_bg.w = 1.0f;
    }

    void paint_menu_shell()
    {
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const ImVec2 min = ImGui::GetWindowPos();
        const ImVec2 max(
            min.x + ImGui::GetWindowWidth(), min.y + ImGui::GetWindowHeight());
        const float r = ImGui::GetStyle().PopupRounding;
        kit_round_fill(draw, min, max, r, ImGui::GetColorU32(g_menu_bg));
        ImVec4 rim = ImGui::GetStyleColorVec4(ImGuiCol_Text);
        rim.w = 0.22f; // same verdict as the dialog shell
        kit_round_border(draw, min, max, r, rim);
    }

    void pop_menu_style()
    {
        ImGui::PopStyleVar(4);
    }

}

bool kit_menu_begin(const char* id)
{
    push_menu_style();
    const bool open = ImGui::BeginPopup(id, ImGuiWindowFlags_NoBackground);
    if (!open) {
        pop_menu_style();
        return false;
    }
    paint_menu_shell();
    ImGui::PushItemFlag(ImGuiItemFlags_NoNav, true);
    return true;
}

void kit_menu_end()
{
    ImGui::PopItemFlag();
    ImGui::EndPopup();
    pop_menu_style();
}

bool kit_menu_item(
    const char* label, const char* trailing, bool selected, bool enabled)
{
    return ImGui::MenuItem(label, trailing, selected, enabled);
}

float kit_menu_row_width(const char* label, const char* trailing)
{
    const float em = ImGui::GetFontSize();
    float w = em * 0.95f + em * 0.55f + ImGui::CalcTextSize(label).x;
    if (trailing && *trailing)
        w += em * 1.6f
            + ImGui::GetFont()
                  ->CalcTextSizeA(kit_caption_size(), FLT_MAX, 0.0f, trailing)
                  .x;
    return w;
}

bool kit_menu_item_icon(const char* swatch_name, const char* label,
    const char* trailing, bool selected, float width)
{
    const float em = ImGui::GetFontSize();
    const float sw = em * 0.95f;
    const float row_h = em * 1.7f;
    if (width <= 0.0f)
        width = kit_menu_row_width(label, trailing);

    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const bool clicked
        = ImGui::Selectable("##row", selected, 0, ImVec2(width, row_h));
    if (ImGui::IsItemHovered())
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

    // The style-editor grammar: a flat color square speaks for the
    // thing, the label names it, the trailing note keeps to the right
    // edge every row shares.
    ImDrawList* draw = ImGui::GetWindowDrawList();
    const ImVec2 sp(kit_snap(pos.x), kit_snap(pos.y + (row_h - sw) * 0.5f));
    kit_round_fill(draw, sp, ImVec2(sp.x + sw, sp.y + sw), sw * 0.22f,
        ImGui::GetColorU32(kit_identity_color(swatch_name)));
    kit_round_border(draw, sp, ImVec2(sp.x + sw, sp.y + sw), sw * 0.22f,
        ImVec4(0, 0, 0, 40.0f / 255.0f), 1.0f);
    draw->AddText(ImVec2(kit_snap(pos.x + sw + em * 0.55f),
                      kit_snap(pos.y + (row_h - em) * 0.5f)),
        ImGui::GetColorU32(ImGuiCol_Text), label);
    if (trailing && *trailing) {
        // The note keeps to the right edge but never crowds the label:
        // whatever space the label leaves (minus a breath) is the
        // note's whole budget, and a long name elides into it.
        const float label_end
            = sw + em * 0.55f + ImGui::CalcTextSize(label).x + em * 0.8f;
        const float budget = width - label_end;
        const std::string shown
            = kit_elide_middle(trailing, budget, kit_caption_size());
        const float trailing_w = ImGui::GetFont()
                                     ->CalcTextSizeA(kit_caption_size(),
                                         FLT_MAX, 0.0f, shown.c_str())
                                     .x;
        draw->AddText(ImGui::GetFont(), kit_caption_size(),
            ImVec2(kit_snap(pos.x + width - trailing_w),
                kit_snap(pos.y + (row_h - kit_caption_size()) * 0.5f)),
            ImGui::GetColorU32(ImGuiCol_TextDisabled), shown.c_str());
    }
    return clicked;
}

}
