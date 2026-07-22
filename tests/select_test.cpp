#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <imgui.h>
#include <imgui_internal.h>

#include <array>
#include <string_view>

#include "ui/widgets/select.hpp"
#include "ui/widgets/text_field.hpp"

namespace {

class ImGuiHarness {
public:
    ImGuiHarness()
    {
        context_ = ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = { 800.0f, 600.0f };
        io.DeltaTime = 1.0f / 60.0f;
        unsigned char* pixels = nullptr;
        int width = 0;
        int height = 0;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    }

    ~ImGuiHarness()
    {
        ImGui::DestroyContext(context_);
    }

    void begin()
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddMousePosEvent(40.0f, 40.0f);
        ImGui::NewFrame();
        ImGui::SetNextWindowPos({ 0.0f, 0.0f }, ImGuiCond_Always);
        ImGui::SetNextWindowSize({ 500.0f, 500.0f }, ImGuiCond_Always);
        ImGui::Begin("select-test-host", nullptr,
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings);
        ImGui::SetCursorScreenPos({ 20.0f, 20.0f });
    }

    void end()
    {
        ImGui::End();
        ImGui::EndFrame();
    }

private:
    ImGuiContext* context_ = nullptr;
};

struct SelectFrame {
    bool open = false;
    bool scrollbar = false;
    float popup_rounding = -1.0f;
    float popup_x = -1.0f;
    float popup_width = -1.0f;
    float popup_padding_x = -1.0f;
};

SelectFrame draw_select()
{
    SelectFrame frame;
    frame.open = izan::ui::kit_select_begin("##chain", "Ethereum", 240.0f);
    if (!frame.open)
        return frame;

    ImGuiWindow* popup = ImGui::GetCurrentWindow();
    frame.popup_rounding = popup->WindowRounding;
    frame.popup_x = popup->Pos.x;
    frame.popup_width = popup->Size.x;
    frame.popup_padding_x = popup->WindowPadding.x;
    frame.scrollbar = popup->ScrollbarY;
    for (const char* label :
        { "Ethereum", "Robinhood Chain", "Base", "Arbitrum One", "Polygon",
            "Sepolia", "Optimism", "Avalanche", "BNB Chain", "Linea" }) {
        izan::ui::kit_select_item(label, label == std::string_view("Ethereum"));
    }
    izan::ui::kit_select_end();
    return frame;
}

}

TEST_CASE("select uses field height without leaking popup style")
{
    ImGuiHarness imgui;
    ImGui::GetStyle().PopupRounding = 9.0f;
    imgui.begin();

    CHECK_FALSE(izan::ui::kit_select_begin("##closed", "Ethereum", 240.0f));
    const float select_height = ImGui::GetItemRectSize().y;

    ImGui::SetCursorScreenPos({ 20.0f, 90.0f });
    ImGui::SetNextItemWidth(240.0f);
    std::array<char, 16> text {};
    izan::ui::kit_text_field("##text", "Ethereum", text.data(), text.size());
    const float field_height = ImGui::GetItemRectSize().y;

    CHECK(select_height == doctest::Approx(field_height));
    CHECK(ImGui::GetStyle().PopupRounding == doctest::Approx(9.0f));
    imgui.end();
}

TEST_CASE("open select delegates its menu to native combo styling")
{
    ImGuiHarness imgui;
    ImGui::GetStyle().PopupRounding = 9.0f;

    imgui.begin();
    const float expected_rounding = ImGui::GetStyle().PopupRounding;
    const float expected_padding = ImGui::GetStyle().FramePadding.x;
    ImGuiContext& context = *ImGui::GetCurrentContext();
    const ImGuiID item_id = ImGui::GetCurrentWindow()->GetID("##chain");
    context.NavActivateId = item_id;
    context.NavActivateDownId = item_id;
    const SelectFrame opening = draw_select();
    REQUIRE(opening.open);
    CHECK(opening.popup_rounding == doctest::Approx(expected_rounding));
    CHECK(ImGui::GetStyle().PopupRounding == doctest::Approx(9.0f));
    imgui.end();

    imgui.begin();
    const SelectFrame settled = draw_select();
    REQUIRE(settled.open);
    CHECK(settled.popup_rounding == doctest::Approx(expected_rounding));
    CHECK(settled.popup_x == doctest::Approx(20.0f));
    CHECK(settled.popup_width == doctest::Approx(240.0f));
    CHECK(settled.popup_padding_x == doctest::Approx(expected_padding));
    CHECK(settled.scrollbar);
    CHECK(ImGui::GetStyle().PopupRounding == doctest::Approx(9.0f));
    imgui.end();
}
