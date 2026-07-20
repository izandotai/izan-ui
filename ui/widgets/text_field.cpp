#include "ui/widgets/text_field.hpp"

#include <cstring>

#include <imgui.h>
#include <sodium.h>

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

    // The recessed twin of the button's four-stroke gloss, mirrored
    // for the same overhead light: the top lip shades the floor, the
    // bottom of the well catches a little back-light, the outer lower
    // lip takes the light that clears the rim, and the rim itself runs
    // dark-on-top. Focus wraps the well in the accent halo.
    void paint_well(const ImVec2& min, const ImVec2& max, bool focused)
    {
        ImDrawList* draw = ImGui::GetWindowDrawList();
        const float em = ImGui::GetFontSize();
        const float r = em * design().field_radius;
        const bool dark = kit_is_dark();
        const float h = max.y - min.y;

        auto clipped = [&](float y0, float y1, auto&& paint) {
            draw->PushClipRect(
                ImVec2(min.x - 4.0f, y0), ImVec2(max.x + 4.0f, y1), true);
            paint();
            draw->PopClipRect();
        };

        // Top inner shadow: three fading inset strokes, corner-true.
        clipped(min.y, min.y + h * 0.45f, [&] {
            for (int i = 1; i <= 3; ++i) {
                const float a = (dark ? 0.17f : 0.09f) * float(4 - i) / 3.0f;
                draw->AddRect(ImVec2(min.x + float(i), min.y + float(i)),
                    ImVec2(max.x - float(i), max.y - float(i)),
                    ImGui::GetColorU32(ImVec4(0, 0, 0, a)),
                    r > float(i) ? r - float(i) : 0.0f, 0, 1.0f);
            }
        });
        // A whisper of back-light on the well's inner floor edge.
        clipped(max.y - h * 0.3f, max.y, [&] {
            draw->AddRect(ImVec2(min.x + 1, min.y + 1),
                ImVec2(max.x - 1, max.y - 1),
                ImGui::GetColorU32(ImVec4(1, 1, 1, dark ? 0.045f : 0.35f)),
                r - 1.0f, 0, 1.0f);
        });
        // The light that clears the rim lands on the outer lower lip.
        clipped(max.y - h * 0.25f, max.y + 2.5f, [&] {
            draw->AddRect(ImVec2(min.x - 0.5f, min.y - 0.5f),
                ImVec2(max.x + 0.5f, max.y + 0.5f),
                ImGui::GetColorU32(ImVec4(1, 1, 1, dark ? 0.07f : 0.55f)),
                r + 0.5f, 0, 1.5f);
        });
        // The rim: brushed chrome. One ring stroke per horizontal
        // band, the color walking a vertical ramp — polished bright
        // along the top edge, falling dark through the waist, a
        // second glint across the base. The band seams vanish into
        // the ramp; the eye reads one machined bezel.
        auto metal = [&](float t) {
            struct Stop {
                float t, l;
            };
            static constexpr Stop ramp[] = { { 0.00f, 0.92f },
                { 0.15f, 0.55f }, { 0.40f, 0.28f }, { 0.65f, 0.34f },
                { 0.85f, 0.60f }, { 1.00f, 0.88f } };
            float l = ramp[0].l;
            for (std::size_t k = 1; k < sizeof(ramp) / sizeof(ramp[0]); ++k) {
                if (t <= ramp[k].t) {
                    const float f
                        = (t - ramp[k - 1].t) / (ramp[k].t - ramp[k - 1].t);
                    l = ramp[k - 1].l + (ramp[k].l - ramp[k - 1].l) * f;
                    break;
                }
                l = ramp[k].l;
            }
            if (!dark)
                l = l * 0.80f + 0.05f; // gentler chrome on a light panel
            const float b = l * 1.06f;
            return ImVec4(l * 0.97f, l, b > 1.0f ? 1.0f : b, 1.0f);
        };
        const int bands = 12;
        const float ring_top = min.y - 1.5f;
        const float ring_h = h + 3.0f;
        for (int i = 0; i < bands; ++i) {
            const float t0 = float(i) / float(bands);
            const float t1 = float(i + 1) / float(bands);
            draw->PushClipRect(ImVec2(min.x - 4.0f, ring_top + ring_h * t0),
                ImVec2(max.x + 4.0f, ring_top + ring_h * t1), true);
            draw->AddRect(min, max,
                ImGui::GetColorU32(metal((t0 + t1) * 0.5f)), r, 0, 2.0f);
            draw->PopClipRect();
        }

        if (focused) {
            ImVec4 halo = kit_accent();
            for (int i = 1; i <= 3; ++i) {
                halo.w = 0.30f / float(i);
                draw->AddRect(ImVec2(min.x - float(i), min.y - float(i)),
                    ImVec2(max.x + float(i), max.y + float(i)),
                    ImGui::GetColorU32(halo), r + float(i), 0, 1.5f);
            }
            ImVec4 ring = kit_accent();
            ring.w = 0.85f;
            draw->AddRect(min, max, ImGui::GetColorU32(ring), r, 0, 2.0f);
        }
    }

}

// Every field in the app wears the same clothes: a generous rounded
// well, recessed where the buttons are raised. The imgui input only
// fills the floor; the well strokes land after the item.
void kit_field_style_push()
{
    const float em = ImGui::GetFontSize();
    ImGui::PushStyleColor(ImGuiCol_FrameBg, well_floor());
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    ImGui::PushStyleVar(
        ImGuiStyleVar_FrameRounding, em * design().field_radius);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
        ImVec2(em * design().field_pad_x, em * design().field_pad_y));
}

void kit_field_style_pop()
{
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
}

// Finish the well over the last item's rectangle.
void kit_field_well_finish(bool focused)
{
    paint_well(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), focused);
}

void kit_field_frame(const ImVec2& pos, const ImVec2& size)
{
    ImDrawList* draw = ImGui::GetWindowDrawList();
    const ImVec2 max(pos.x + size.x, pos.y + size.y);
    const float r = ImGui::GetFontSize() * design().field_radius;
    draw->AddRectFilled(pos, max, ImGui::GetColorU32(well_floor()), r);
    paint_well(pos, max, false);
}

bool kit_text_field(
    const char* id, const char* hint, char* buf, std::size_t size)
{
    kit_field_style_push();
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
    const bool changed = ImGui::InputTextMultiline(id, buf, size,
        ImVec2(-1.0f, ImGui::GetTextLineHeight() * rows + em * 0.9f));
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
