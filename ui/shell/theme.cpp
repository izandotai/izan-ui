#include "ui/shell/theme.hpp"

#include <algorithm>
#include <cmath>

namespace izan::ui {

void apply_style()
{
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 6.0f;
    style.ChildRounding = 4.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.TabRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.WindowPadding = ImVec2(10.0f, 8.0f);
    style.FramePadding = ImVec2(10.0f, 5.0f);
    style.ItemSpacing = ImVec2(8.0f, 6.0f);

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.105f, 0.110f, 0.135f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.075f, 0.078f, 0.095f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.110f, 0.116f, 0.145f, 0.98f);
    colors[ImGuiCol_Border] = ImVec4(0.270f, 0.285f, 0.350f, 0.75f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.155f, 0.165f, 0.205f, 0.94f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.215f, 0.230f, 0.285f, 0.96f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.255f, 0.275f, 0.345f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.090f, 0.095f, 0.120f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.120f, 0.125f, 0.155f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.090f, 0.095f, 0.120f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.100f, 0.105f, 0.130f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.250f, 0.145f, 0.330f, 0.80f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.330f, 0.205f, 0.430f, 0.90f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.410f, 0.255f, 0.520f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.155f, 0.165f, 0.205f, 0.88f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.245f, 0.255f, 0.315f, 0.95f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.325f, 0.210f, 0.440f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.130f, 0.138f, 0.172f, 0.95f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.320f, 0.205f, 0.430f, 0.95f);
    colors[ImGuiCol_TabActive] = ImVec4(0.205f, 0.155f, 0.285f, 1.00f);
    colors[ImGuiCol_DockingPreview] = ImVec4(0.550f, 0.360f, 0.760f, 0.70f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.075f, 0.078f, 0.095f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.560f, 0.360f, 0.760f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.680f, 0.450f, 0.900f, 1.00f);
}

void set_theme_rounding(ImGuiStyle& style, float radius)
{
    style.WindowRounding = radius;
    style.ChildRounding = radius * 0.65f;
    style.FrameRounding = radius * 0.55f;
    style.PopupRounding = radius;
    style.TabRounding = radius * 0.65f;
    style.GrabRounding = radius * 0.55f;
    style.ScrollbarRounding = radius * 0.45f;
}

void set_theme_spacing(ImGuiStyle& style, float window_padding,
    float frame_padding, float item_spacing)
{
    style.WindowPadding = ImVec2(window_padding, window_padding * 0.8f);
    style.FramePadding = ImVec2(frame_padding, frame_padding * 0.6f);
    style.ItemSpacing = ImVec2(item_spacing, item_spacing * 0.75f);
}

void apply_nord_style()
{
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    set_theme_rounding(style, 6.0f);
    set_theme_spacing(style, 10.0f, 6.0f, 8.0f);
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.ScrollbarSize = 14.0f;
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = ImVec4(0.847f, 0.871f, 0.914f, 1.0f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.506f, 0.631f, 0.757f, 1.0f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.180f, 0.204f, 0.251f, 1.0f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.150f, 0.170f, 0.210f, 1.0f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.208f, 0.235f, 0.294f, 0.98f);
    colors[ImGuiCol_Border] = ImVec4(0.299f, 0.337f, 0.416f, 0.80f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.208f, 0.235f, 0.294f, 0.95f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.231f, 0.263f, 0.322f, 1.0f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.267f, 0.310f, 0.380f, 1.0f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.150f, 0.170f, 0.210f, 1.0f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.208f, 0.235f, 0.294f, 1.0f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.150f, 0.170f, 0.210f, 1.0f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.150f, 0.170f, 0.210f, 1.0f);
    colors[ImGuiCol_Header] = ImVec4(0.369f, 0.506f, 0.675f, 0.55f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.529f, 0.753f, 0.816f, 0.55f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.529f, 0.753f, 0.816f, 0.80f);
    colors[ImGuiCol_Button] = ImVec4(0.208f, 0.235f, 0.294f, 0.90f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.369f, 0.506f, 0.675f, 0.75f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.529f, 0.753f, 0.816f, 0.85f);
    colors[ImGuiCol_Tab] = ImVec4(0.208f, 0.235f, 0.294f, 0.95f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.369f, 0.506f, 0.675f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(0.267f, 0.310f, 0.380f, 1.0f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.529f, 0.753f, 0.816f, 1.0f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.529f, 0.753f, 0.816f, 1.0f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.706f, 0.557f, 0.678f, 1.0f);
    colors[ImGuiCol_DockingPreview] = ImVec4(0.529f, 0.753f, 0.816f, 0.55f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.150f, 0.170f, 0.210f, 1.0f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.369f, 0.506f, 0.675f, 0.35f);
}

void apply_claude_style()
{
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    set_theme_rounding(style, 8.0f);
    set_theme_spacing(style, 12.0f, 7.0f, 8.0f);
    style.WindowBorderSize = 0.0f;
    style.FrameBorderSize = 0.0f;
    style.ScrollbarSize = 14.0f;
    ImVec4* colors = style.Colors;
    const ImVec4 bg0(0.102f, 0.098f, 0.086f, 1.0f);
    const ImVec4 bg1(0.141f, 0.137f, 0.122f, 0.98f);
    const ImVec4 bg2(0.184f, 0.180f, 0.161f, 1.0f);
    const ImVec4 coral(0.855f, 0.467f, 0.337f, 1.0f);
    colors[ImGuiCol_Text] = ImVec4(0.953f, 0.937f, 0.898f, 1.0f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.580f, 0.557f, 0.510f, 1.0f);
    colors[ImGuiCol_WindowBg] = bg0;
    colors[ImGuiCol_ChildBg] = ImVec4(0.086f, 0.082f, 0.071f, 1.0f);
    colors[ImGuiCol_PopupBg] = bg1;
    colors[ImGuiCol_Border] = ImVec4(0.250f, 0.240f, 0.210f, 0.80f);
    colors[ImGuiCol_FrameBg] = bg1;
    colors[ImGuiCol_FrameBgHovered] = bg2;
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.220f, 0.220f, 0.190f, 1.0f);
    colors[ImGuiCol_TitleBg] = bg0;
    colors[ImGuiCol_TitleBgActive] = bg1;
    colors[ImGuiCol_TitleBgCollapsed] = bg0;
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.086f, 0.082f, 0.071f, 1.0f);
    colors[ImGuiCol_Header] = ImVec4(coral.x, coral.y, coral.z, 0.25f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(coral.x, coral.y, coral.z, 0.50f);
    colors[ImGuiCol_HeaderActive] = ImVec4(coral.x, coral.y, coral.z, 0.75f);
    colors[ImGuiCol_Button] = bg1;
    colors[ImGuiCol_ButtonHovered] = ImVec4(coral.x, coral.y, coral.z, 0.70f);
    colors[ImGuiCol_ButtonActive] = coral;
    colors[ImGuiCol_Tab] = bg1;
    colors[ImGuiCol_TabHovered] = bg2;
    colors[ImGuiCol_TabActive] = ImVec4(coral.x, coral.y, coral.z, 0.60f);
    colors[ImGuiCol_CheckMark] = coral;
    colors[ImGuiCol_SliderGrab] = ImVec4(coral.x, coral.y, coral.z, 0.80f);
    colors[ImGuiCol_SliderGrabActive] = coral;
    colors[ImGuiCol_DockingPreview] = ImVec4(coral.x, coral.y, coral.z, 0.50f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.086f, 0.082f, 0.071f, 1.0f);
    colors[ImGuiCol_TableHeaderBg] = bg1;
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.220f, 0.210f, 0.180f, 0.50f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.300f, 0.290f, 0.250f, 0.80f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(coral.x, coral.y, coral.z, 0.35f);
    colors[ImGuiCol_SeparatorHovered] = coral;
    colors[ImGuiCol_SeparatorActive] = coral;
}

void apply_light_style()
{
    ImGui::StyleColorsLight();
    ImGuiStyle& style = ImGui::GetStyle();
    set_theme_rounding(style, 6.0f);
    set_theme_spacing(style, 10.0f, 6.0f, 8.0f);
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.ScrollbarSize = 14.0f;
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.965f, 0.973f, 0.984f, 1.0f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.945f, 0.953f, 0.965f, 1.0f);
    colors[ImGuiCol_PopupBg] = ImVec4(1.000f, 1.000f, 1.000f, 0.98f);
    colors[ImGuiCol_Border] = ImVec4(0.710f, 0.745f, 0.780f, 0.85f);
    colors[ImGuiCol_FrameBg] = ImVec4(1.000f, 1.000f, 1.000f, 0.95f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.902f, 0.929f, 0.965f, 1.0f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.820f, 0.878f, 0.949f, 1.0f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.902f, 0.922f, 0.945f, 1.0f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.820f, 0.878f, 0.949f, 1.0f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.902f, 0.922f, 0.945f, 1.0f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.922f, 0.941f, 0.965f, 1.0f);
    colors[ImGuiCol_Header] = ImVec4(0.035f, 0.412f, 0.855f, 0.18f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.035f, 0.412f, 0.855f, 0.30f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.035f, 0.412f, 0.855f, 0.40f);
    colors[ImGuiCol_Button] = ImVec4(0.902f, 0.929f, 0.965f, 0.95f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.820f, 0.878f, 0.949f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.660f, 0.790f, 0.930f, 1.0f);
    colors[ImGuiCol_Tab] = ImVec4(0.902f, 0.922f, 0.945f, 0.95f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.820f, 0.878f, 0.949f, 1.0f);
    colors[ImGuiCol_TabActive] = ImVec4(1.000f, 1.000f, 1.000f, 1.0f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.035f, 0.412f, 0.855f, 1.0f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.035f, 0.412f, 0.855f, 0.85f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.035f, 0.300f, 0.620f, 1.0f);
    colors[ImGuiCol_DockingPreview] = ImVec4(0.035f, 0.412f, 0.855f, 0.45f);
    colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.945f, 0.953f, 0.965f, 1.0f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.035f, 0.412f, 0.855f, 0.22f);
}

void apply_custom_style(const ImVec4& text, const ImVec4& text_disabled,
    const ImVec4& window, const ImVec4& child, const ImVec4& popup,
    const ImVec4& frame, const ImVec4& frame_hovered,
    const ImVec4& frame_active, const ImVec4& accent, const ImVec4& title,
    bool light)
{
    if (light)
        ImGui::StyleColorsLight();
    else
        ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    set_theme_rounding(style, 6.0f);
    set_theme_spacing(style, 10.0f, 6.0f, 8.0f);
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
    style.ScrollbarSize = 14.0f;
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = text;
    colors[ImGuiCol_TextDisabled] = text_disabled;
    colors[ImGuiCol_WindowBg] = window;
    colors[ImGuiCol_ChildBg] = child;
    colors[ImGuiCol_PopupBg] = popup;
    colors[ImGuiCol_Border] = ImVec4(text_disabled.x, text_disabled.y,
        text_disabled.z, light ? 0.45f : 0.75f);
    colors[ImGuiCol_FrameBg] = frame;
    colors[ImGuiCol_FrameBgHovered] = frame_hovered;
    colors[ImGuiCol_FrameBgActive] = frame_active;
    colors[ImGuiCol_TitleBg] = title;
    colors[ImGuiCol_TitleBgActive] = frame_active;
    colors[ImGuiCol_TitleBgCollapsed] = title;
    colors[ImGuiCol_MenuBarBg] = title;
    colors[ImGuiCol_Header]
        = ImVec4(accent.x, accent.y, accent.z, light ? 0.18f : 0.38f);
    colors[ImGuiCol_HeaderHovered]
        = ImVec4(accent.x, accent.y, accent.z, light ? 0.30f : 0.58f);
    colors[ImGuiCol_HeaderActive]
        = ImVec4(accent.x, accent.y, accent.z, light ? 0.42f : 0.78f);
    colors[ImGuiCol_Button] = frame;
    colors[ImGuiCol_ButtonHovered] = frame_hovered;
    colors[ImGuiCol_ButtonActive]
        = ImVec4(accent.x, accent.y, accent.z, light ? 0.46f : 0.85f);
    colors[ImGuiCol_Tab] = title;
    colors[ImGuiCol_TabHovered]
        = ImVec4(accent.x, accent.y, accent.z, light ? 0.28f : 0.58f);
    colors[ImGuiCol_TabActive] = frame;
    colors[ImGuiCol_CheckMark] = accent;
    colors[ImGuiCol_SliderGrab] = ImVec4(accent.x, accent.y, accent.z, 0.85f);
    colors[ImGuiCol_SliderGrabActive] = accent;
    colors[ImGuiCol_DockingPreview]
        = ImVec4(accent.x, accent.y, accent.z, 0.45f);
    colors[ImGuiCol_DockingEmptyBg] = child;
    colors[ImGuiCol_TextSelectedBg]
        = ImVec4(accent.x, accent.y, accent.z, light ? 0.22f : 0.35f);
    colors[ImGuiCol_SeparatorHovered] = accent;
    colors[ImGuiCol_SeparatorActive] = accent;
}

void apply_indexed_theme_style(int theme_index)
{
    switch (theme_index) {
    case 5: // One Dark
        apply_custom_style(ImVec4(0.671f, 0.698f, 0.749f, 1.0f),
            ImVec4(0.435f, 0.475f, 0.545f, 1.0f),
            ImVec4(0.157f, 0.169f, 0.208f, 1.0f),
            ImVec4(0.126f, 0.137f, 0.169f, 1.0f),
            ImVec4(0.190f, 0.204f, 0.251f, 0.98f),
            ImVec4(0.190f, 0.204f, 0.251f, 0.95f),
            ImVec4(0.247f, 0.267f, 0.329f, 1.0f),
            ImVec4(0.298f, 0.322f, 0.396f, 1.0f),
            ImVec4(0.380f, 0.686f, 0.937f, 1.0f),
            ImVec4(0.126f, 0.137f, 0.169f, 1.0f), false);
        return;
    case 6: // Monokai
        apply_custom_style(ImVec4(0.973f, 0.973f, 0.949f, 1.0f),
            ImVec4(0.459f, 0.447f, 0.369f, 1.0f),
            ImVec4(0.153f, 0.150f, 0.125f, 1.0f),
            ImVec4(0.117f, 0.113f, 0.094f, 1.0f),
            ImVec4(0.200f, 0.196f, 0.165f, 0.98f),
            ImVec4(0.200f, 0.196f, 0.165f, 0.95f),
            ImVec4(0.290f, 0.282f, 0.235f, 1.0f),
            ImVec4(0.360f, 0.345f, 0.282f, 1.0f),
            ImVec4(0.651f, 0.886f, 0.180f, 1.0f),
            ImVec4(0.117f, 0.113f, 0.094f, 1.0f), false);
        return;
    case 7: // Gruvbox Dark
        apply_custom_style(ImVec4(0.922f, 0.859f, 0.698f, 1.0f),
            ImVec4(0.573f, 0.514f, 0.455f, 1.0f),
            ImVec4(0.157f, 0.157f, 0.157f, 1.0f),
            ImVec4(0.118f, 0.118f, 0.118f, 1.0f),
            ImVec4(0.235f, 0.219f, 0.188f, 0.98f),
            ImVec4(0.235f, 0.219f, 0.188f, 0.95f),
            ImVec4(0.314f, 0.275f, 0.227f, 1.0f),
            ImVec4(0.408f, 0.345f, 0.278f, 1.0f),
            ImVec4(0.839f, 0.600f, 0.129f, 1.0f),
            ImVec4(0.118f, 0.118f, 0.118f, 1.0f), false);
        return;
    case 8: // Gruvbox Light
        apply_custom_style(ImVec4(0.235f, 0.219f, 0.188f, 1.0f),
            ImVec4(0.573f, 0.514f, 0.455f, 1.0f),
            ImVec4(0.984f, 0.945f, 0.800f, 1.0f),
            ImVec4(0.922f, 0.859f, 0.698f, 1.0f),
            ImVec4(0.984f, 0.945f, 0.800f, 0.98f),
            ImVec4(0.922f, 0.859f, 0.698f, 0.95f),
            ImVec4(0.835f, 0.753f, 0.604f, 1.0f),
            ImVec4(0.749f, 0.659f, 0.510f, 1.0f),
            ImVec4(0.686f, 0.384f, 0.071f, 1.0f),
            ImVec4(0.922f, 0.859f, 0.698f, 1.0f), true);
        return;
    case 9: // Solarized Dark
        apply_custom_style(ImVec4(0.514f, 0.580f, 0.588f, 1.0f),
            ImVec4(0.345f, 0.431f, 0.459f, 1.0f),
            ImVec4(0.000f, 0.169f, 0.212f, 1.0f),
            ImVec4(0.027f, 0.212f, 0.259f, 1.0f),
            ImVec4(0.027f, 0.212f, 0.259f, 0.98f),
            ImVec4(0.027f, 0.212f, 0.259f, 0.95f),
            ImVec4(0.035f, 0.278f, 0.337f, 1.0f),
            ImVec4(0.035f, 0.337f, 0.408f, 1.0f),
            ImVec4(0.149f, 0.545f, 0.824f, 1.0f),
            ImVec4(0.000f, 0.169f, 0.212f, 1.0f), false);
        return;
    case 10: // Solarized Light
        apply_custom_style(ImVec4(0.396f, 0.482f, 0.514f, 1.0f),
            ImVec4(0.576f, 0.631f, 0.631f, 1.0f),
            ImVec4(0.992f, 0.965f, 0.890f, 1.0f),
            ImVec4(0.933f, 0.910f, 0.835f, 1.0f),
            ImVec4(0.992f, 0.965f, 0.890f, 0.98f),
            ImVec4(0.933f, 0.910f, 0.835f, 0.95f),
            ImVec4(0.878f, 0.843f, 0.737f, 1.0f),
            ImVec4(0.820f, 0.773f, 0.647f, 1.0f),
            ImVec4(0.149f, 0.545f, 0.824f, 1.0f),
            ImVec4(0.933f, 0.910f, 0.835f, 1.0f), true);
        return;
    case 11: // Tokyo Night
        apply_custom_style(ImVec4(0.753f, 0.792f, 0.961f, 1.0f),
            ImVec4(0.431f, 0.463f, 0.620f, 1.0f),
            ImVec4(0.094f, 0.102f, 0.176f, 1.0f),
            ImVec4(0.075f, 0.082f, 0.149f, 1.0f),
            ImVec4(0.126f, 0.137f, 0.220f, 0.98f),
            ImVec4(0.126f, 0.137f, 0.220f, 0.95f),
            ImVec4(0.169f, 0.188f, 0.290f, 1.0f),
            ImVec4(0.220f, 0.243f, 0.365f, 1.0f),
            ImVec4(0.478f, 0.635f, 0.980f, 1.0f),
            ImVec4(0.075f, 0.082f, 0.149f, 1.0f), false);
        return;
    case 12: // Catppuccin Mocha
        apply_custom_style(ImVec4(0.804f, 0.839f, 0.957f, 1.0f),
            ImVec4(0.584f, 0.604f, 0.753f, 1.0f),
            ImVec4(0.118f, 0.110f, 0.169f, 1.0f),
            ImVec4(0.094f, 0.090f, 0.133f, 1.0f),
            ImVec4(0.180f, 0.169f, 0.243f, 0.98f),
            ImVec4(0.180f, 0.169f, 0.243f, 0.95f),
            ImVec4(0.243f, 0.224f, 0.322f, 1.0f),
            ImVec4(0.302f, 0.278f, 0.392f, 1.0f),
            ImVec4(0.796f, 0.651f, 0.969f, 1.0f),
            ImVec4(0.094f, 0.090f, 0.133f, 1.0f), false);
        return;
    case 13: // GitHub Dark
        apply_custom_style(ImVec4(0.788f, 0.827f, 0.878f, 1.0f),
            ImVec4(0.545f, 0.600f, 0.671f, 1.0f),
            ImVec4(0.051f, 0.067f, 0.090f, 1.0f),
            ImVec4(0.008f, 0.027f, 0.051f, 1.0f),
            ImVec4(0.086f, 0.106f, 0.137f, 0.98f),
            ImVec4(0.086f, 0.106f, 0.137f, 0.95f),
            ImVec4(0.129f, 0.157f, 0.204f, 1.0f),
            ImVec4(0.180f, 0.212f, 0.263f, 1.0f),
            ImVec4(0.173f, 0.490f, 0.831f, 1.0f),
            ImVec4(0.008f, 0.027f, 0.051f, 1.0f), false);
        return;
    case 14: // GitHub Light
        apply_custom_style(ImVec4(0.141f, 0.161f, 0.184f, 1.0f),
            ImVec4(0.341f, 0.376f, 0.416f, 1.0f),
            ImVec4(1.000f, 1.000f, 1.000f, 1.0f),
            ImVec4(0.965f, 0.973f, 0.984f, 1.0f),
            ImVec4(1.000f, 1.000f, 1.000f, 0.98f),
            ImVec4(0.965f, 0.973f, 0.984f, 0.95f),
            ImVec4(0.918f, 0.941f, 0.969f, 1.0f),
            ImVec4(0.820f, 0.878f, 0.949f, 1.0f),
            ImVec4(0.035f, 0.412f, 0.855f, 1.0f),
            ImVec4(0.965f, 0.973f, 0.984f, 1.0f), true);
        return;
    default:
        return;
    }
}

void make_popup_background_opaque()
{
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_PopupBg].w = 1.0f;
}

ImVec4 theme_accent_color(int theme_index)
{
    switch (theme_index) {
    case 1:
        return ImVec4(0.741f, 0.576f, 0.976f, 1.0f);
    case 2:
        return ImVec4(0.855f, 0.467f, 0.337f, 1.0f);
    case 3:
        return ImVec4(0.529f, 0.753f, 0.816f, 1.0f);
    case 4:
        return ImVec4(0.035f, 0.412f, 0.855f, 1.0f);
    case 5:
        return ImVec4(0.380f, 0.686f, 0.937f, 1.0f);
    case 6:
        return ImVec4(0.651f, 0.886f, 0.180f, 1.0f);
    case 7:
    case 8:
        return ImVec4(0.839f, 0.600f, 0.129f, 1.0f);
    case 9:
    case 10:
        return ImVec4(0.149f, 0.545f, 0.824f, 1.0f);
    case 11:
        return ImVec4(0.478f, 0.635f, 0.980f, 1.0f);
    case 12:
        return ImVec4(0.796f, 0.651f, 0.969f, 1.0f);
    case 13:
    case 14:
        return ImVec4(0.173f, 0.490f, 0.831f, 1.0f);
    default:
        return ImVec4(0.560f, 0.360f, 0.760f, 1.0f);
    }
}

ImVec4 theme_ok_color(int theme_index)
{
    switch (theme_index) {
    case 2:
        return ImVec4(0.400f, 0.780f, 0.510f, 1.0f);
    case 3:
        return ImVec4(0.639f, 0.745f, 0.549f, 1.0f);
    case 4:
    case 8:
    case 10:
    case 14:
        return ImVec4(0.100f, 0.550f, 0.220f, 1.0f);
    default:
        return ImVec4(0.314f, 0.980f, 0.482f, 1.0f);
    }
}

ImVec4 theme_info_color(int theme_index)
{
    switch (theme_index) {
    case 2:
        return ImVec4(0.580f, 0.557f, 0.510f, 1.0f);
    case 3:
        return ImVec4(0.506f, 0.631f, 0.757f, 1.0f);
    case 4:
    case 8:
    case 10:
    case 14:
        return ImVec4(0.330f, 0.360f, 0.400f, 1.0f);
    default:
        return ImVec4(0.650f, 0.650f, 0.720f, 1.0f);
    }
}

void apply_theme_style_only(int theme_index)
{
    if (theme_index == 2) {
        apply_claude_style();
    } else if (theme_index == 3) {
        apply_nord_style();
    } else if (theme_index == 4) {
        apply_light_style();
    } else if (theme_index >= 5) {
        apply_indexed_theme_style(theme_index);
    } else {
        apply_style();
        if (theme_index == 1) {
            ImGuiStyle& style = ImGui::GetStyle();
            style.Colors[ImGuiCol_WindowBg]
                = ImVec4(0.157f, 0.165f, 0.212f, 1.0f);
            style.Colors[ImGuiCol_ChildBg]
                = ImVec4(0.122f, 0.125f, 0.157f, 1.0f);
            style.Colors[ImGuiCol_PopupBg]
                = ImVec4(0.267f, 0.278f, 0.353f, 0.98f);
            style.Colors[ImGuiCol_TitleBg]
                = ImVec4(0.120f, 0.125f, 0.157f, 1.0f);
            style.Colors[ImGuiCol_TitleBgActive]
                = ImVec4(0.267f, 0.278f, 0.353f, 1.0f);
            style.Colors[ImGuiCol_TitleBgCollapsed]
                = ImVec4(0.120f, 0.125f, 0.157f, 1.0f);
            style.Colors[ImGuiCol_MenuBarBg]
                = ImVec4(0.120f, 0.125f, 0.157f, 1.0f);
            style.Colors[ImGuiCol_Header]
                = ImVec4(0.741f, 0.576f, 0.976f, 0.40f);
            style.Colors[ImGuiCol_HeaderHovered]
                = ImVec4(0.741f, 0.576f, 0.976f, 0.70f);
            style.Colors[ImGuiCol_HeaderActive]
                = ImVec4(0.741f, 0.576f, 0.976f, 0.90f);
            style.Colors[ImGuiCol_ButtonActive]
                = ImVec4(0.741f, 0.576f, 0.976f, 1.0f);
            style.Colors[ImGuiCol_TabActive]
                = ImVec4(0.267f, 0.278f, 0.353f, 1.0f);
            style.Colors[ImGuiCol_DockingPreview]
                = ImVec4(0.741f, 0.576f, 0.976f, 0.55f);
            style.Colors[ImGuiCol_DockingEmptyBg]
                = ImVec4(0.122f, 0.125f, 0.157f, 1.0f);
            style.Colors[ImGuiCol_TextSelectedBg]
                = ImVec4(0.741f, 0.576f, 0.976f, 0.35f);
        }
    }
    make_popup_background_opaque();
}

ImU32 color_u32_from_vec4(const ImVec4& color)
{
    return ImGui::ColorConvertFloat4ToU32(color);
}

bool is_light_theme_index(int theme_index)
{
    return theme_index == 4 || theme_index == 8 || theme_index == 10
        || theme_index == 14;
}

ImU32 theme_frame_background_color(const ChromeState& app)
{
    switch (app.theme_index) {
    case 1:
        return IM_COL32(40, 42, 54, 238);
    case 2:
        return IM_COL32(26, 25, 22, 238);
    case 3:
        return IM_COL32(46, 52, 64, 238);
    case 4:
        return IM_COL32(246, 248, 250, 248);
    case 8:
        return IM_COL32(251, 241, 204, 255);
    case 10:
        return IM_COL32(253, 246, 227, 255);
    case 14:
        return IM_COL32(255, 255, 255, 255);
    default:
        if (app.theme_index >= 5) {
            ImVec4 frame = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
            frame.w = 1.0f;
            return color_u32_from_vec4(frame);
        }
        return IM_COL32(24, 25, 34, 238);
    }
}

ImU32 theme_chrome_background_color(const ChromeState& app)
{
    switch (app.theme_index) {
    case 1:
        return IM_COL32(31, 32, 42, 248);
    case 2:
        return IM_COL32(22, 21, 18, 248);
    case 3:
        return IM_COL32(36, 41, 51, 248);
    case 4:
        return IM_COL32(226, 232, 240, 248);
    case 8:
        return IM_COL32(235, 219, 178, 255);
    case 10:
        return IM_COL32(238, 232, 213, 255);
    case 14:
        return IM_COL32(246, 248, 250, 255);
    default:
        if (app.theme_index >= 5) {
            ImVec4 title = ImGui::GetStyleColorVec4(ImGuiCol_TitleBg);
            title.w = 1.0f;
            return color_u32_from_vec4(title);
        }
        return IM_COL32(22, 23, 31, 248);
    }
}

ImU32 theme_chrome_line_color(const ChromeState& app, int alpha)
{
    const ImVec4 accent = theme_accent_color(app.theme_index);
    return color_u32_from_vec4(ImVec4(
        accent.x, accent.y, accent.z, static_cast<float>(alpha) / 255.0f));
}

ImU32 theme_transparent_chrome_window_color(const ChromeState& app)
{
    (void)app;
    return IM_COL32(0, 0, 0, 0);
}

ImVec4 theme_panel_window_color(const ChromeState& app)
{
    switch (app.theme_index) {
    case 1:
        return ImVec4(0.157f, 0.165f, 0.212f, 1.0f);
    case 2:
        return ImVec4(0.102f, 0.098f, 0.086f, 1.0f);
    case 3:
        return ImVec4(0.180f, 0.204f, 0.251f, 1.0f);
    case 4:
        return ImVec4(0.965f, 0.973f, 0.984f, 1.0f);
    case 8:
        return ImVec4(0.984f, 0.945f, 0.800f, 1.0f);
    case 10:
        return ImVec4(0.992f, 0.965f, 0.890f, 1.0f);
    case 14:
        return ImVec4(1.000f, 1.000f, 1.000f, 1.0f);
    default:
        if (app.theme_index >= 5)
            return ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
        return ImVec4(0.105f, 0.110f, 0.135f, 1.0f);
    }
}

ImVec4 theme_panel_child_color(const ChromeState& app)
{
    switch (app.theme_index) {
    case 1:
        return ImVec4(0.122f, 0.125f, 0.157f, 1.0f);
    case 2:
        return ImVec4(0.086f, 0.082f, 0.071f, 1.0f);
    case 3:
        return ImVec4(0.150f, 0.170f, 0.210f, 1.0f);
    case 4:
        return ImVec4(0.945f, 0.953f, 0.965f, 1.0f);
    case 8:
        return ImVec4(0.922f, 0.859f, 0.698f, 1.0f);
    case 10:
        return ImVec4(0.933f, 0.910f, 0.835f, 1.0f);
    case 14:
        return ImVec4(0.965f, 0.973f, 0.984f, 1.0f);
    default:
        if (app.theme_index >= 5)
            return ImGui::GetStyleColorVec4(ImGuiCol_ChildBg);
        return ImVec4(0.075f, 0.078f, 0.095f, 1.0f);
    }
}

void draw_themed_panel_content_background(const ChromeState& app)
{
    const ImVec2 window_pos = ImGui::GetWindowPos();
    const ImVec2 content_min = ImGui::GetWindowContentRegionMin();
    const ImVec2 content_max = ImGui::GetWindowContentRegionMax();
    const ImVec2 min(
        window_pos.x + content_min.x, window_pos.y + content_min.y);
    const ImVec2 max(
        window_pos.x + content_max.x, window_pos.y + content_max.y);
    ImGui::GetWindowDrawList()->AddRectFilled(min, max,
        ImGui::ColorConvertFloat4ToU32(theme_panel_window_color(app)));
}

ImU32 theme_snap_preview_border_color(const ChromeState& app, bool hovered)
{
    const ImVec4 accent = theme_accent_color(app.theme_index);
    return color_u32_from_vec4(
        ImVec4(accent.x, accent.y, accent.z, hovered ? 1.0f : 0.65f));
}

ImU32 theme_snap_preview_fill_color(const ChromeState& app, bool hovered)
{
    const ImVec4 accent = theme_accent_color(app.theme_index);
    return color_u32_from_vec4(
        ImVec4(accent.x, accent.y, accent.z, hovered ? 0.36f : 0.13f));
}

ImU32 theme_snap_preview_background_color(const ChromeState& app)
{
    const ImVec4 frame = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
    return color_u32_from_vec4(ImVec4(frame.x, frame.y, frame.z,
        is_light_theme_index(app.theme_index) ? 0.75f : 0.38f));
}

ImU32 theme_popup_shadow_color(const ChromeState& app, float alpha)
{
    const float adjusted_alpha
        = is_light_theme_index(app.theme_index) ? alpha * 0.65f : alpha * 0.78f;
    return IM_COL32(0, 0, 0,
        static_cast<int>(std::clamp(adjusted_alpha, 0.0f, 1.0f) * 255.0f));
}

ImVec4 theme_clear_color(const ChromeState& app)
{
    switch (app.theme_index) {
    case 1:
        return ImVec4(0.157f, 0.165f, 0.212f, 1.0f);
    case 2:
        return ImVec4(0.102f, 0.098f, 0.086f, 1.0f);
    case 3:
        return ImVec4(0.180f, 0.204f, 0.251f, 1.0f);
    case 4:
        return ImVec4(0.965f, 0.973f, 0.984f, 1.0f);
    case 8:
        return ImVec4(0.984f, 0.945f, 0.800f, 1.0f);
    case 10:
        return ImVec4(0.992f, 0.965f, 0.890f, 1.0f);
    case 14:
        return ImVec4(1.000f, 1.000f, 1.000f, 1.0f);
    default:
        if (app.theme_index >= 5) {
            ImVec4 clear = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
            clear.w = 1.0f;
            return clear;
        }
        return ImVec4(0.105f, 0.110f, 0.135f, 1.0f);
    }
}

}
