#pragma once

#include <imgui.h>

#include <string>

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

    // Called inside the window's content child every frame the
    // window is visible.
    virtual void draw() = 0;
};

}
