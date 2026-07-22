#pragma once

#include <imgui.h>

#include <string_view>
#include <utility>
#include <vector>

#include "ui/os/app.hpp"
#include "ui/os/panel.hpp"
#include "ui/os/theme.hpp"
#include "ui/os/wm.hpp"

namespace izan::os {

// The composition root: wallpaper below, windows in the middle, the
// panel above — one frame() inside whatever rectangle the host
// chrome hands over. Apps stay owned by the host.
class Shell {
public:
    void attach(App* app);

    // The uninstall half of the registry: the window shuts and the
    // app leaves the launcher roster and window table. The object stays
    // with the host; GPU-backed final destruction waits until the frame's
    // renderer submission even when detach is requested from App::draw().
    void detach(App* app);

    // User close intents collected during the last frame. Draining does not
    // detach or destroy anything; instance policy belongs to the host.
    std::vector<CloseRequest> take_close_requests()
    {
        return wm_.take_close_requests();
    }

    void set_catalog(std::vector<AppDescriptor> catalog)
    {
        panel_.set_catalog(std::move(catalog));
    }

    void request_launch(std::string_view id)
    {
        panel_.request_launch(id);
    }

    std::vector<LaunchRequest> take_launch_requests()
    {
        return panel_.take_launch_requests();
    }

    void set_theme(const Theme* theme)
    {
        wm_.set_theme(theme);
    }

    // The emoji the launcher button wears.
    void set_mark(const char* mark)
    {
        mark_ = mark;
    }

    // A pre-baked wallpaper: when set, the shell blits this texture
    // instead of running the theme's painter every frame. The UVs
    // default to a GL render-target's row order (bottom-up).
    void set_wallpaper(ImTextureID texture, ImVec2 uv0 = { 0.0f, 1.0f },
        ImVec2 uv1 = { 1.0f, 0.0f })
    {
        wallpaper_ = texture;
        wallpaper_uv0_ = uv0;
        wallpaper_uv1_ = uv1;
    }

    void frame(ImVec2 pos, ImVec2 size);

    Wm& wm()
    {
        return wm_;
    }

    Panel& panel()
    {
        return panel_;
    }

private:
    std::vector<App*> apps_;
    Wm wm_;
    Panel panel_;
    const char* mark_ = "⛩️";
    ImTextureID wallpaper_ = 0;
    ImVec2 wallpaper_uv0_ { 0.0f, 1.0f };
    ImVec2 wallpaper_uv1_ { 1.0f, 0.0f };
};

}
