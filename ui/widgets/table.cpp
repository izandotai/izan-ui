#include "ui/widgets/table.hpp"

#include <imgui.h>

#include "ui/widgets/label.hpp"

namespace izan::ui {

bool kit_table_begin(const char* id, int columns)
{
    return ImGui::BeginTable(id, columns,
        ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH
            | ImGuiTableFlags_SizingStretchProp);
}

void kit_table_headers(const char* const* names, int count)
{
    ImGui::PushFont(nullptr, kit_caption_size());
    for (int i = 0; i < count; ++i)
        ImGui::TableSetupColumn(names[i]);
    ImGui::TableHeadersRow();
    ImGui::PopFont();
}

void kit_table_end()
{
    ImGui::EndTable();
}

}
