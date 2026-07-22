#include "ui/widgets/text_field.hpp"

#include <cstring>

#include <imgui.h>
#include <sodium.h>

#include "ui/render/sdf_rect.hpp"
#include "ui/widgets/design.hpp"

namespace izan::ui {

namespace {

    // The well's floor: recessed below the window in the dark themes
    // (a slot milled into the panel), near-white in the light ones
    // where the shadows alone carry the depth.
    ImVec4 well_floor()
    {
        const ImVec4 bg = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
        if (kit_is_dark())
            return kit_blend(bg, ImVec4(0, 0, 0, 1), 0.22f);
        return kit_blend(bg, ImVec4(1, 1, 1, 1), 0.55f);
    }

    // Rings route through the kit door: language gauge and alpha.
    void sdf_ring(ImDrawList* draw, ImVec2 min, ImVec2 max, float r,
        const ImVec4& color, float px)
    {
        kit_round_border(draw, min, max, r, color, px);
    }

    // ONE visible frame — the component contract. The old recessed
    // lighting (three inner shadows, back-light, outer lip) stacked
    // five near-coincident rings on a large radius and read as one
    // muddy band; the 2026-07-20 side-by-side against the mint input
    // (fill + single border, clean) settled it. Focus still speaks
    // with the accent halo, drawn outside the silhouette.
    void paint_well(const ImVec2& min, const ImVec2& max, bool focused)
    {
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const float em = ImGui::GetFontSize();
        const float r = em * design().field_radius;

        sdf_ring(draw, min, max, r,
            ImGui::GetStyleColorVec4(ImGuiCol_Separator), design().border_px);

        if (focused) {
            ImVec4 halo = kit_accent();
            for (int i = 1; i <= 3; ++i) {
                halo.w = 0.30f / float(i);
                sdf_ring(draw, ImVec2(min.x - float(i), min.y - float(i)),
                    ImVec2(max.x + float(i), max.y + float(i)), r + float(i),
                    halo, 1.5f);
            }
            ImVec4 ring = kit_accent();
            ring.w = 0.85f;
            sdf_ring(draw, min, max, r, ring, design().border_px);
        }
    }

    // The floor, laid analytically before the (transparent) imgui item
    // — the library's own polygon-arc frame never shows again.
    void paint_floor(const ImVec2& pos, const ImVec2& size)
    {
        render::SdfRect floor;
        floor.min = pos;
        floor.max = ImVec2(pos.x + size.x, pos.y + size.y);
        const float r = ImGui::GetFontSize() * design().field_radius;
        floor.radius[0] = floor.radius[1] = floor.radius[2] = floor.radius[3]
            = r;
        floor.fill = ImGui::GetColorU32(well_floor());
        render::sdf_rect(ImGui::GetWindowDrawList(), floor);
    }

}

// Every field in the app wears the same clothes: a generous rounded
// well, recessed where the buttons are raised. The imgui input only
// fills the floor; the well strokes land after the item.
void kit_field_style_push()
{
    const float em = ImGui::GetFontSize();
    // Transparent frame: the floor is laid analytically beforehand,
    // imgui's polygon-arc fill stays out of the picture.
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
    // Keyboard nav paints its own cursor ring just outside the frame;
    // the well already speaks focus with the accent halo — one voice.
    ImGui::PushStyleColor(ImGuiCol_NavCursor, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    ImGui::PushStyleVar(
        ImGuiStyleVar_FrameRounding, em * design().field_radius);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
        ImVec2(em * design().field_pad_x, em * design().field_pad_y));
}

void kit_field_style_pop()
{
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(3);
}

// Finish the well over the last item's rectangle.
void kit_field_well_finish(bool focused)
{
    paint_well(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), focused);
}

void kit_field_frame(const ImVec2& pos, const ImVec2& size)
{
    kit_field_frame(pos, size, false);
}

void kit_field_frame(const ImVec2& pos, const ImVec2& size, bool focused)
{
    const ImVec2 max(pos.x + size.x, pos.y + size.y);
    paint_floor(pos, size);
    paint_well(pos, max, focused);
}

bool kit_text_field(
    const char* id, const char* hint, char* buf, std::size_t size)
{
    kit_field_style_push();
    // Frame metrics only settle after the style push (padding rides
    // in it); the floor must measure the same box the item will.
    paint_floor(ImGui::GetCursorScreenPos(),
        ImVec2(ImGui::CalcItemWidth(), ImGui::GetFrameHeight()));
    const bool submitted = ImGui::InputTextWithHint(
        id, hint, buf, size, ImGuiInputTextFlags_EnterReturnsTrue);
    kit_field_style_pop();
    kit_field_well_finish(ImGui::IsItemActive());
    return submitted;
}

bool secret_field(const char* label, std::array<char, 256>& buf,
    bool& secret_focus, const char* hint)
{
    constexpr ImGuiInputTextFlags kFlags = ImGuiInputTextFlags_Password
        | ImGuiInputTextFlags_AutoSelectAll
        | ImGuiInputTextFlags_EnterReturnsTrue;
    kit_field_style_push();
    paint_floor(ImGui::GetCursorScreenPos(),
        ImVec2(ImGui::CalcItemWidth(), ImGui::GetFrameHeight()));
    const bool submitted = hint
        ? ImGui::InputTextWithHint(label, hint, buf.data(), buf.size(), kFlags)
        : ImGui::InputText(label, buf.data(), buf.size(), kFlags);
    kit_field_style_pop();
    const bool active = ImGui::IsItemActive();
    kit_field_well_finish(active);
    secret_focus |= active;
    return submitted;
}

bool kit_paste_box(const char* id, const char* hint, char* buf,
    std::size_t size, float rows, bool& secret_focus)
{
    const float em = ImGui::GetFontSize();
    kit_field_style_push();
    ImGui::PushStyleVar(
        ImGuiStyleVar_FramePadding, ImVec2(em * 0.55f, em * 0.45f));
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const ImVec2 box(ImGui::GetContentRegionAvail().x,
        ImGui::GetTextLineHeight() * rows + em * 0.9f);
    paint_floor(pos, box);
    const bool changed
        = ImGui::InputTextMultiline(id, buf, size, ImVec2(-1.0f, box.y));
    const bool active = ImGui::IsItemActive();
    ImGui::PopStyleVar();
    kit_field_style_pop();
    kit_field_well_finish(active);
    secret_focus |= active;

    // The hint, painted while the box is empty — multiline inputs have
    // no built-in one.
    if (hint && *hint && buf[0] == '\0' && !active)
        ImGui::GetWindowDrawList()->AddText(
            ImVec2(pos.x + em * 0.55f, pos.y + em * 0.45f),
            ImGui::GetColorU32(ImGuiCol_TextDisabled), hint);
    return changed;
}

void kit_focus_here()
{
    ImGui::SetKeyboardFocusHere();
    ImGui::SetNavCursorVisible(false);
}

secure::SecureBytes take_secret(std::array<char, 256>& buf)
{
    const std::size_t len = strnlen(buf.data(), buf.size());
    secure::SecureBytes out(len);
    if (len)
        std::memcpy(out.data(), buf.data(), len);
    sodium_memzero(buf.data(), buf.size());
    return out;
}

}
