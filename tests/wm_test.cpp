#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <imgui.h>

#include <algorithm>
#include <functional>
#include <string>
#include <vector>

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
            last_draw_pos = ImGui::GetWindowPos();
            last_draw_size = ImGui::GetWindowSize();
            drew = true;
            if (on_draw)
                on_draw();
        }

        std::function<void()> on_draw;
        ImVec2 last_draw_pos {};
        ImVec2 last_draw_size {};
        bool drew = false;

    private:
        const char* id_;
    };

    class MultiWindowApp final : public izan::os::App {
    public:
        const char* id() const override { return "multi-probe"; }
        const char* name() const override { return "Multi Probe"; }
        const char* mark() const override { return "M"; }
        std::vector<izan::os::AppWindowSpec> secondary_windows() const override
        {
            return { { "inspector", "Inspector", { 24.0f, 16.0f } },
                { "timeline", "Timeline", { 28.0f, 14.0f } } };
        }
        std::vector<izan::os::AppWindowCommand> take_window_commands() override
        {
            std::vector<izan::os::AppWindowCommand> out;
            out.swap(commands);
            return out;
        }
        void draw() override { draw_window(izan::os::kMainWindowId); }
        void draw_window(std::string_view window_id) override
        {
            drawn.emplace_back(window_id);
            ImGui::TextUnformatted(window_id.data(),
                window_id.data() + window_id.size());
        }

        std::vector<izan::os::AppWindowCommand> commands;
        std::vector<std::string> drawn;
    };

    class ImGuiHarness {
    public:
        ImGuiHarness()
        {
            context_ = ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.DisplaySize = { 1280.0f, 720.0f };
            io.DeltaTime = 1.0f / 60.0f;
            io.IniFilename = nullptr;
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

TEST_CASE("a replacement instance with the same id restores window placement")
{
    ImGuiHarness imgui;
    izan::os::Wm wm;
    TestApp first("placement-probe");
    wm.attach(&first);
    imgui.prime(wm);
    wm.launch(&first);
    imgui.prime(wm);
    REQUIRE(first.drew);
    const ImVec2 original = first.last_draw_pos;

    wm.request_close(&first);
    wm.detach(&first);
    TestApp replacement("placement-probe");
    wm.attach(&replacement);
    wm.launch(&replacement);
    imgui.prime(wm);

    REQUIRE(replacement.drew);
    CHECK(replacement.last_draw_pos.x == doctest::Approx(original.x));
    CHECK(replacement.last_draw_pos.y == doctest::Approx(original.y));

    TestApp unseen("new-placement-probe");
    wm.attach(&unseen);
    wm.launch(&unseen);
    imgui.prime(wm);
    REQUIRE(unseen.drew);
    CHECK(unseen.last_draw_pos.x > original.x);
    CHECK(unseen.last_draw_pos.y > original.y);
}

TEST_CASE("stable placements cross a window-manager process boundary")
{
    ImGuiHarness first_imgui;
    izan::os::Wm first_wm;
    izan::os::WindowPlacement seeded { { 310.0f, 125.0f }, { 460.0f, 330.0f },
        { 280.0f, 110.0f }, { 420.0f, 300.0f }, false };
    REQUIRE(first_wm.restore_placement("session-probe", seeded));
    CHECK_FALSE(first_wm.restore_placement("", seeded));
    izan::os::WindowPlacement broken = seeded;
    broken.size.x = -1.0f;
    CHECK_FALSE(first_wm.restore_placement("broken", broken));

    TestApp first("session-probe");
    first_wm.attach(&first);
    first_imgui.prime(first_wm);
    first_wm.launch(&first);
    first_imgui.prime(first_wm);
    REQUIRE(first.drew);

    const auto snapshot = first_wm.snapshot_placements();
    REQUIRE(snapshot.size() == 1);
    CHECK(snapshot.front().id == "session-probe");
    CHECK(snapshot.front().placement.pos.x == doctest::Approx(seeded.pos.x));
    CHECK(snapshot.front().placement.pos.y == doctest::Approx(seeded.pos.y));
    CHECK(snapshot.front().placement.size.x == doctest::Approx(seeded.size.x));
    CHECK(snapshot.front().placement.size.y == doctest::Approx(seeded.size.y));

    izan::os::Wm replacement_wm;
    REQUIRE(replacement_wm.restore_placement(
        snapshot.front().id, snapshot.front().placement));
    TestApp replacement("session-probe");
    replacement_wm.attach(&replacement);
    first_imgui.prime(replacement_wm);
    replacement_wm.launch(&replacement);
    first_imgui.prime(replacement_wm);
    CHECK(
        replacement.last_draw_pos.x == doctest::Approx(first.last_draw_pos.x));
    CHECK(
        replacement.last_draw_pos.y == doctest::Approx(first.last_draw_pos.y));
    CHECK(replacement.last_draw_size.x
        == doctest::Approx(first.last_draw_size.x));
    CHECK(replacement.last_draw_size.y
        == doctest::Approx(first.last_draw_size.y));
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

TEST_CASE("one app owns distinct windows and close intents stay window-local")
{
    ImGuiHarness imgui;
    izan::os::Wm wm;
    MultiWindowApp app;
    wm.attach(&app);
    imgui.prime(wm);

    wm.launch(&app);
    wm.launch(&app, "inspector");
    wm.launch(&app, "timeline");
    REQUIRE(wm.open_windows(&app).size() == 3);
    CHECK(wm.running(&app, "main"));
    CHECK(wm.running(&app, "inspector"));

    wm.request_close(&app, "inspector");
    wm.request_close(&app, "inspector");
    wm.request_close(&app, "timeline");
    CHECK(wm.running(&app));
    CHECK_FALSE(wm.running(&app, "inspector"));
    auto requests = wm.take_close_requests();
    REQUIRE(requests.size() == 2);
    CHECK(requests[0].window_id == "inspector");
    CHECK(requests[1].window_id == "timeline");

    wm.request_close(&app);
    CHECK_FALSE(wm.running(&app));
    requests = wm.take_close_requests();
    REQUIRE(requests.size() == 1);
    CHECK(requests.front().window_id == "main");
}

TEST_CASE("app window commands cross the frame boundary safely")
{
    ImGuiHarness imgui;
    izan::os::Wm wm;
    MultiWindowApp app;
    wm.attach(&app);
    wm.launch(&app);
    app.commands.push_back(
        { izan::os::AppWindowCommandKind::Open, "inspector" });
    imgui.prime(wm);

    CHECK(wm.running(&app, "main"));
    CHECK(wm.running(&app, "inspector"));
    CHECK(std::find(app.drawn.begin(), app.drawn.end(), "main")
        != app.drawn.end());
    CHECK(std::find(app.drawn.begin(), app.drawn.end(), "inspector")
        != app.drawn.end());

    app.commands.push_back(
        { izan::os::AppWindowCommandKind::Close, "inspector" });
    imgui.prime(wm);
    CHECK_FALSE(wm.running(&app, "inspector"));
    const auto requests = wm.take_close_requests();
    REQUIRE(requests.size() == 1);
    CHECK(requests.front().window_id == "inspector");
}

TEST_CASE("secondary window placements have collision-free stable keys")
{
    ImGuiHarness imgui;
    izan::os::Wm wm;
    MultiWindowApp app;
    wm.attach(&app);
    imgui.prime(wm);
    wm.launch(&app);
    wm.launch(&app, "inspector");
    imgui.prime(wm);

    const auto placements = wm.snapshot_placements();
    REQUIRE(placements.size() == 2);
    const auto secondary = std::find_if(placements.begin(), placements.end(),
        [](const auto& record) { return record.window_id == "inspector"; });
    REQUIRE(secondary != placements.end());
    CHECK(secondary->id == "multi-probe~inspector");
    CHECK(secondary->app_id == "multi-probe");
    CHECK(secondary->window_id == "inspector");
}
