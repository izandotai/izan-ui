#include "ui/widgets/select.hpp"

#include <imgui.h>

namespace izan::ui {

bool kit_select_begin(const char* id, const char* preview, float width)
{
    if (width > 0.0f)
        ImGui::SetNextItemWidth(width);
    return ImGui::BeginCombo(id, preview);
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
