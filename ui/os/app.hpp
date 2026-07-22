#pragma once

#include <imgui.h>

#include <string>
#include <string_view>
#include <vector>

// izan OS: a desktop-grade windowing kernel inside the izan chrome.
// Not an operating system — a window manager, a dock, a menu bar and
// an app registry, all compile-time C++. Every product in the family
// ships as an App inside this shell.

namespace izan::os {

// Installation metadata is a value, not a running App. The launcher may list
// these records without forcing the host to construct an application instance.
struct AppDescriptor {
    std::string id;
    std::string name;
    std::string mark;
};

struct LaunchRequest {
    std::string id;
};

inline constexpr std::string_view kMainWindowId = "main";

// Secondary windows are declared by stable, application-local ids. The main
// window remains implicit so every existing App keeps exactly its old ABI and
// behaviour. A host may cap or reject malformed declarations.
struct AppWindowSpec {
    std::string id;
    std::string title;
    ImVec2 initial_size_em { 30.0f, 22.0f };
};

enum class AppWindowCommandKind { Open, Close };

struct AppWindowCommand {
    AppWindowCommandKind kind = AppWindowCommandKind::Open;
    std::string id;
};

// One program: identity plus a content painter. The window manager
// owns geometry and lifecycle; the app only paints its content
// region (real widgets, laid out in ems — never a scaled picture).
class App {
public:
    virtual ~App() = default;

    virtual const char* id() const = 0;   // stable ascii identifier
    virtual const char* name() const = 0; // dock tooltip, title bar
    virtual const char* mark() const = 0; // one emoji, the app's face

    // Content size in ems at spawn; the user resizes from there.
    virtual ImVec2 initial_size_em() const
    {
        return { 30.0f, 22.0f };
    }

    // Optional static secondary-window contract. The host snapshots this at
    // attach time; ids and bounds are validated before any window is created.
    virtual std::vector<AppWindowSpec> secondary_windows() const
    {
        return {};
    }

    // App-originated window actions are drained by Wm at a safe frame edge.
    // Existing applications need no queue and therefore return nothing.
    virtual std::vector<AppWindowCommand> take_window_commands()
    {
        return {};
    }

    // Called inside the window's content child every frame the
    // window is visible.
    virtual void draw() = 0;

    // Per-window painter. The compatibility implementation gives secondary
    // windows the main content; multi-window apps override this dispatcher.
    virtual void draw_window(std::string_view window_id)
    {
        (void)window_id;
        draw();
    }
};

}
