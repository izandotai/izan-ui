#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <imgui.h>

#include <functional>

#include "ui/os/shell.hpp"
#include "ui/os/wm.hpp"

namespace {

    class TestApp final : public izan::os::App {
    public:
        explicit TestApp(const char* app_id)
            : id_(app_id)
        {
        }

        const char* id() const override { return id_; }
        const char* name() const override { return id_; }
        const char* mark() const override { return "T"; }
        void draw() override
        {
            if (on_draw)
                on_draw();
        }

        std::function<void()> on_draw;

    private:
        const char* id_;
    };

    class ImGuiHarness {
    public:
        ImGuiHarness()
        {
            context_ = ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.DisplaySize = { 1280.0f, 720.0f };
            io.DeltaTime = 1.0f / 60.0f;
            unsigned char* pixels = nullptr;
            int width = 0, height = 0;
            io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
        }

        ~ImGuiHarness()
        {
            ImGui::DestroyContext(context_);
        }

        void prime(izan::os::Wm& wm)
        {
            ImGui::NewFrame();
            wm.frame({ 0.0f, 0.0f }, { 1280.0f, 680.0f }, {});
            ImGui::EndFrame();
        }

    private:
        ImGuiContext* context_ = nullptr;
    };

}

TEST_CASE("a user close is edge-triggered and a host close is silent")
{
    ImGuiHarness imgui;
    izan::os::Wm wm;
    TestApp app("close-probe");
    wm.attach(&app);
    imgui.prime(wm);

    wm.launch(&app);
    REQUIRE(wm.running(&app));
    wm.request_close(&app);
    CHECK_FALSE(wm.running(&app));
    wm.request_close(&app);

    auto requests = wm.take_close_requests();
    REQUIRE(requests.size() == 1);
    CHECK(requests.front().app == &app);
    CHECK(wm.take_close_requests().empty());

    wm.launch(&app);
    REQUIRE(wm.running(&app));
    wm.close(&app);
    CHECK_FALSE(wm.running(&app));
    CHECK(wm.take_close_requests().empty());
}

TEST_CASE("detach repairs the compact window table and discards stale intents")
{
    ImGuiHarness imgui;
    izan::os::Wm wm;
    TestApp first("first");
    TestApp middle("middle");
    TestApp last("last");
    wm.attach(&first);
    wm.attach(&middle);
    wm.attach(&last);
    imgui.prime(wm);

    wm.launch(&first);
    wm.launch(&middle);
    wm.launch(&last);
    REQUIRE(wm.focused() == &last);

    wm.request_close(&middle);
    wm.detach(&middle);
    CHECK_FALSE(wm.attached(&middle));
    CHECK(wm.running(&first));
    CHECK(wm.running(&last));
    CHECK(wm.focused() == &last);
    CHECK(wm.take_close_requests().empty());

    wm.close(&last);
    CHECK(wm.focused() == &first);
    wm.detach(&first);
    CHECK_FALSE(wm.attached(&first));
    CHECK(wm.attached(&last));

    wm.launch(&last);
    CHECK(wm.running(&last));
    CHECK(wm.focused() == &last);
}

TEST_CASE("detach withdraws pending launches and reattach starts fresh")
{
    ImGuiHarness imgui;
    izan::os::Wm wm;
    TestApp app("pending-probe");
    wm.attach(&app);
    wm.launch(&app);
    wm.launch(&app);
    wm.detach(&app);

    CHECK_FALSE(wm.attached(&app));
    imgui.prime(wm);
    CHECK_FALSE(wm.running(&app));

    wm.attach(&app);
    REQUIRE(wm.attached(&app));
    wm.launch(&app);
    CHECK(wm.running(&app));
}

TEST_CASE("shell detach removes both roster and window state")
{
    izan::os::Shell shell;
    TestApp app("shell-probe");

    shell.attach(nullptr);
    CHECK_FALSE(shell.wm().attached(nullptr));

    shell.attach(&app);
    REQUIRE(shell.wm().attached(&app));
    shell.detach(&app);
    CHECK_FALSE(shell.wm().attached(&app));

    shell.attach(&app);
    CHECK(shell.wm().attached(&app));
}

TEST_CASE("installed catalog launch requests are deduplicated and destructive")
{
    izan::os::Shell shell;
    shell.set_catalog({ { "cold-app", "Cold App", "C" } });

    shell.request_launch("cold-app");
    shell.request_launch("cold-app");
    shell.request_launch("");
    auto requests = shell.take_launch_requests();
    REQUIRE(requests.size() == 1);
    CHECK(requests.front().id == "cold-app");
    CHECK(shell.take_launch_requests().empty());
}

TEST_CASE("detach during paint leaves the frame index snapshot valid")
{
    ImGuiHarness imgui;
    izan::os::Wm wm;
    TestApp first("paint-first");
    TestApp middle("paint-middle");
    TestApp last("paint-last");
    wm.attach(&first);
    wm.attach(&middle);
    wm.attach(&last);
    imgui.prime(wm);
    wm.launch(&first);
    wm.launch(&middle);
    wm.launch(&last);

    int last_draws = 0;
    first.on_draw = [&] { wm.detach(&middle); };
    last.on_draw = [&] { ++last_draws; };
    imgui.prime(wm);

    CHECK_FALSE(wm.attached(&middle));
    CHECK(wm.running(&first));
    CHECK(wm.running(&last));
    CHECK(wm.focused() == &last);
    CHECK(last_draws == 1);
}
