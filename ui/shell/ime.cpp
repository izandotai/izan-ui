#include "ui/shell/ime.hpp"

#include "ui/shell/fonts.hpp"

#include <imgui_internal.h>

#include <GLFW/glfw3.h>
#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <imm.h>
#include <windows.h>
#endif

#include <algorithm>
#include <cstdio>

namespace izan::ui {

void update_ime_position(GLFWwindow* window, const ImVec2* override_pos)
{
#ifdef _WIN32
    HWND hwnd = glfwGetWin32Window(window);
    if (hwnd == nullptr)
        return;

    HIMC context = ImmGetContext(hwnd);
    if (context == nullptr)
        return;

    ImVec2 input_pos = ImGui::GetMousePos();
    bool has_input_pos = false;
    if (override_pos != nullptr) {
        input_pos = *override_pos;
        has_input_pos = true;
    } else if (ImGuiContext* imgui_context = ImGui::GetCurrentContext()) {
        const ImGuiPlatformImeData& ime_data = imgui_context->PlatformImeData;
        if (ime_data.WantTextInput || ime_data.WantVisible) {
            input_pos = ime_data.InputPos;
            has_input_pos = true;
        }
    }

    if (!has_input_pos) {
        ImmReleaseContext(hwnd, context);
        return;
    }

    const LONG x = static_cast<LONG>(std::max(0.0f, input_pos.x));
    const LONG y = static_cast<LONG>(std::max(0.0f, input_pos.y));

    COMPOSITIONFORM composition {};
    composition.dwStyle = CFS_POINT;
    composition.ptCurrentPos = POINT { x, y };
    ImmSetCompositionWindow(context, &composition);

    LOGFONTA composition_font {};
    composition_font.lfHeight = -static_cast<LONG>(kDefaultFontSize);
    composition_font.lfWeight = FW_NORMAL;
    composition_font.lfCharSet = DEFAULT_CHARSET;
    composition_font.lfQuality = CLEARTYPE_QUALITY;
    std::snprintf(
        composition_font.lfFaceName, LF_FACESIZE, "%s", kDefaultFontFaceName);
    ImmSetCompositionFontA(context, &composition_font);

    CANDIDATEFORM candidate {};
    candidate.dwIndex = 0;
    candidate.dwStyle = CFS_CANDIDATEPOS;
    candidate.ptCurrentPos
        = POINT { x, y + static_cast<LONG>(ImGui::GetTextLineHeight()) };
    ImmSetCandidateWindow(context, &candidate);

    ImmReleaseContext(hwnd, context);
#else
    (void)window;
    (void)override_pos;
#endif
}

void set_ime_enabled(GLFWwindow* window, bool enabled)
{
#ifdef _WIN32
    // ImmAssociateContext(hwnd, null) detaches but cannot restore;
    // IACE_DEFAULT reattaches the thread's default context.
    HWND hwnd = glfwGetWin32Window(window);
    if (hwnd == nullptr)
        return;
    if (enabled)
        ImmAssociateContextEx(hwnd, nullptr, IACE_DEFAULT);
    else
        ImmAssociateContext(hwnd, nullptr);
#else
    (void)window;
    (void)enabled;
#endif
}

}
