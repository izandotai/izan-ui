#include "ui/os/wm.hpp"

#include <algorithm>
#include <string>

#include <imgui_internal.h>

namespace izan::os {

namespace {

    bool inside(ImVec2 p, ImVec2 min, ImVec2 max)
    {
        return p.x >= min.x && p.y >= min.y && p.x <= max.x && p.y <= max.y;
    }

}

void Wm::attach(App* app)
{
    if (index_of(app) >= 0)
        return;
    WindowState w;
    w.app = app;
    windows_.push_back(w);
}

void Wm::launch(App* app)
{
    if (!framed_) {
        pending_.push_back(app);
        return;
    }
    const int index = index_of(app);
    if (index < 0)
        return;
    WindowState& w = windows_[static_cast<std::size_t>(index)];
    if (!w.open) {
        const float em = ImGui::GetFontSize();
        const ImVec2 want = w.app->initial_size_em();
        w.size = { want.x * em, want.y * em };
        const float step = em * 1.6f;
        const float slot = static_cast<float>(spawn_count_ % 5);
        w.pos = { ws_min_.x + em * 4.0f + slot * step,
            ws_min_.y + em * 2.2f + slot * step };
        w.restore_pos = w.pos;
        w.restore_size = w.size;
        w.open = true;
        w.maximized = false;
        w.spawn_focus = true;
        ++spawn_count_;
        z_.push_back(index);
    }
    w.minimized = false;
    raise(index);
}

void Wm::toggle(App* app)
{
    const int index = index_of(app);
    if (index < 0)
        return;
    WindowState& w = windows_[static_cast<std::size_t>(index)];
    if (!w.open) {
        launch(app);
        return;
    }
    if (w.minimized) {
        w.minimized = false;
        w.spawn_focus = true;
        raise(index);
        return;
    }
    if (focused() == app) {
        w.minimized = true;
        return;
    }
    w.spawn_focus = true;
    raise(index);
}

int Wm::index_of(const App* app) const
{
    for (std::size_t i = 0; i < windows_.size(); ++i)
        if (windows_[i].app == app)
            return static_cast<int>(i);
    return -1;
}

void Wm::raise(int index)
{
    const auto it = std::find(z_.begin(), z_.end(), index);
    if (it != z_.end())
        z_.erase(it);
    z_.push_back(index);
}

App* Wm::focused() const
{
    for (auto it = z_.rbegin(); it != z_.rend(); ++it) {
        const WindowState& w = windows_[static_cast<std::size_t>(*it)];
        if (w.open && !w.minimized)
            return w.app;
    }
    return nullptr;
}

bool Wm::running(const App* app) const
{
    const int i = index_of(app);
    return i >= 0 && windows_[static_cast<std::size_t>(i)].open;
}

bool Wm::minimized(const App* app) const
{
    const int i = index_of(app);
    return i >= 0 && windows_[static_cast<std::size_t>(i)].minimized;
}

void Wm::frame(ImVec2 ws_min, ImVec2 ws_max, const std::vector<OsRect>& blocked)
{
    ws_min_ = ws_min;
    ws_max_ = ws_max;
    framed_ = true;
    if (!pending_.empty()) {
        const std::vector<App*> queued = std::move(pending_);
        pending_.clear();
        for (App* app : queued)
            launch(app);
    }
    const float em = ImGui::GetFontSize();
    const Theme& look = theme();
    const float title_h = look.title_height(em);
    ImGuiIO& io = ImGui::GetIO();

    // ---- input pre-pass: same-frame drag and resize ----
    if (drag_ >= 0) {
        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            drag_ = -1;
        } else {
            WindowState& w = windows_[static_cast<std::size_t>(drag_)];
            // Absolute: the window sits under the same point of its
            // title the whole drag, so a clamp never desyncs it.
            w.pos.x = std::clamp(io.MousePos.x - grab_offset_.x,
                ws_min.x - w.size.x + em * 6.0f, ws_max.x - em * 6.0f);
            w.pos.y = std::clamp(
                io.MousePos.y - grab_offset_.y, ws_min.y, ws_max.y - title_h);
        }
    } else if (resize_ >= 0) {
        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            resize_ = -1;
        } else {
            WindowState& w = windows_[static_cast<std::size_t>(resize_)];
            w.size.x = std::max(
                em * 16.0f, grab_size_.x + (io.MousePos.x - grab_mouse_.x));
            w.size.y = std::max(
                em * 10.0f, grab_size_.y + (io.MousePos.y - grab_mouse_.y));
        }
    } else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)
        && std::none_of(blocked.begin(), blocked.end(),
            [&](const OsRect& r) { return r.contains(io.MousePos); })) {
        for (auto it = z_.rbegin(); it != z_.rend(); ++it) {
            // raise() rewrites z_, which this loop is walking — the
            // index must be taken off the iterator before that, or a
            // background hit reads a shifted slot and grabs the wrong
            // window.
            const int index = *it;
            WindowState& w = windows_[static_cast<std::size_t>(index)];
            if (!w.open || w.minimized)
                continue;
            const ImVec2 rmin = w.maximized ? ws_min : w.pos;
            const ImVec2 rmax = w.maximized
                ? ws_max
                : ImVec2 { w.pos.x + w.size.x, w.pos.y + w.size.y };
            if (!inside(io.MousePos, rmin, rmax))
                continue;
            raise(index);
            bool on_control = false;
            for (int c = 0; c < 3; ++c)
                if (look.control_rect(
                            static_cast<WindowControl>(c), rmin, rmax, em)
                        .contains(io.MousePos))
                    on_control = true;
            const bool in_grip = !w.maximized
                && io.MousePos.x > rmax.x - em * 1.2f
                && io.MousePos.y > rmax.y - em * 1.2f;
            if (in_grip) {
                resize_ = index;
                grab_size_ = w.size;
                grab_mouse_ = io.MousePos;
            } else if (!w.maximized && !on_control
                && io.MousePos.y < rmin.y + title_h) {
                drag_ = index;
                grab_offset_
                    = { io.MousePos.x - w.pos.x, io.MousePos.y - w.pos.y };
            }
            break; // the topmost hit consumes the click
        }
    }

    // ---- paint pass, bottom → top ----
    const std::vector<int> order = z_;
    for (int index : order) {
        WindowState& w = windows_[static_cast<std::size_t>(index)];
        if (w.open && !w.minimized)
            paint_window(index);
    }
}

void Wm::paint_window(int index)
{
    WindowState& w = windows_[static_cast<std::size_t>(index)];
    const float em = ImGui::GetFontSize();
    const Theme& look = theme();
    const float title_h = look.title_height(em);
    const bool focused_win = focused() == w.app;

    const ImVec2 rmin = w.maximized ? ws_min_ : w.pos;
    const ImVec2 rmax = w.maximized
        ? ws_max_
        : ImVec2 { w.pos.x + w.size.x, w.pos.y + w.size.y };
    const ImVec2 rsize { rmax.x - rmin.x, rmax.y - rmin.y };

    // The ImGui window is the logical rect plus a shadow ring, so the
    // theme can paint outside the body without a second window.
    const float margin = em * 1.4f;
    ImGui::SetNextWindowPos({ rmin.x - margin, rmin.y - margin });
    ImGui::SetNextWindowSize(
        { rsize.x + margin * 2.0f, rsize.y + margin * 2.0f });
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(0, 0, 0, 0));
    // NoBringToFrontOnFocus: a click must not let imgui hoist the
    // window above the panel for a frame — the kernel re-asserts the
    // whole display order below, every window, every frame.
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings
        | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
        | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBringToFrontOnFocus;
    const std::string name = std::string("###os-window-") + w.app->id();
    ImGui::Begin(name.c_str(), nullptr, flags);
    if (w.spawn_focus) {
        ImGui::SetWindowFocus();
        w.spawn_focus = false;
    }
    // Painted bottom -> top: fronting each window in turn leaves the
    // display order exactly the kernel's z order.
    ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());

    WindowLook wl;
    wl.app = w.app;
    wl.focused = focused_win;
    wl.maximized = w.maximized;
    look.paint_window(ImGui::GetWindowDrawList(), wl, rmin, rmax, em);

    // Control actions live in the kernel; the theme only says where.
    for (int c = 0; c < 3; ++c) {
        const OsRect r
            = look.control_rect(static_cast<WindowControl>(c), rmin, rmax, em);
        if (r.max.x <= r.min.x)
            continue;
        ImGui::SetCursorScreenPos(r.min);
        const std::string bid
            = std::string("##control-") + w.app->id() + std::to_string(c);
        if (ImGui::InvisibleButton(
                bid.c_str(), { r.max.x - r.min.x, r.max.y - r.min.y })) {
            if (c == static_cast<int>(WindowControl::Close)) {
                w.open = false;
                const auto it = std::find(z_.begin(), z_.end(), index);
                if (it != z_.end())
                    z_.erase(it);
            } else if (c == static_cast<int>(WindowControl::Minimize)) {
                w.minimized = true;
            } else if (!w.maximized) {
                w.restore_pos = w.pos;
                w.restore_size = w.size;
                w.maximized = true;
            } else {
                w.pos = w.restore_pos;
                w.size = w.restore_size;
                w.maximized = false;
            }
        }
    }

    {
        // The content region: real widgets, clipped to the body. Drawn
        // even on the frame a control just closed or minimized the
        // window — skipping it leaves one frame of hollow chrome, a
        // visible blink on the way out.
        ImGui::SetCursorScreenPos({ rmin.x + 1.0f, rmin.y + title_h + 1.0f });
        // The child must be glass-clear: the imgui theme's ChildBg is
        // an opaque slab that would sit between the theme's body and
        // the app's tints, blacking out the window.
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleVar(
            ImGuiStyleVar_WindowPadding, ImVec2(em * 0.9f, em * 0.7f));
        const std::string content = std::string("##os-content-") + w.app->id();
        ImGui::BeginChild(content.c_str(),
            { rsize.x - 2.0f, rsize.y - title_h - 2.0f }, ImGuiChildFlags_None,
            ImGuiWindowFlags_NoSavedSettings);
        w.app->draw();
        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
    }

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

}
