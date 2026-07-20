#include "ui/widgets/design.hpp"

#include "ui/render/sdf_rect.hpp"

namespace izan::ui {

void kit_round_fill(
    ImDrawList* draw, ImVec2 min, ImVec2 max, float radius, ImU32 color)
{
    render::SdfRect body;
    body.min = min;
    body.max = max;
    body.radius[0] = body.radius[1] = body.radius[2] = body.radius[3]
        = radius > 0.0f ? radius : 0.0f;
    body.fill = color;
    render::sdf_rect(draw, body);
}

void kit_round_border(ImDrawList* draw, ImVec2 min, ImVec2 max, float radius,
    ImVec4 color, float px)
{
    render::SdfRect ring;
    ring.min = min;
    ring.max = max;
    ring.radius[0] = ring.radius[1] = ring.radius[2] = ring.radius[3]
        = radius > 0.0f ? radius : 0.0f;
    color.w *= design().border_alpha;
    ring.border = ImGui::GetColorU32(color);
    ring.border_px = px > 0.0f ? px : design().border_px;
    render::sdf_rect(draw, ring);
}

namespace {

    DesignLanguage g_design = design_cupertino();

}

const DesignLanguage& design()
{
    return g_design;
}

void set_design(const DesignLanguage& language)
{
    g_design = language;
}

DesignLanguage design_cupertino()
{
    // The defaults in the struct ARE Cupertino — the language the
    // wallet ships with.
    return DesignLanguage {};
}

ImVec4 kit_blend(const ImVec4& a, const ImVec4& b, float t)
{
    return ImVec4(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t);
}

bool kit_is_dark()
{
    const ImVec4 bg = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
    return 0.2126f * bg.x + 0.7152f * bg.y + 0.0722f * bg.z < 0.5f;
}

ImVec4 kit_accent()
{
    return ImGui::GetStyleColorVec4(ImGuiCol_CheckMark);
}

ImVec4 kit_danger()
{
    // A firm red, nudged toward the theme's text so it sits in-palette.
    return kit_blend(ImVec4(0.86f, 0.26f, 0.24f, 1.0f),
        ImGui::GetStyleColorVec4(ImGuiCol_Text), 0.08f);
}

}
