#pragma once

#include <imgui.h>

#include <vector>

#include "ui/os/app.hpp"
#include "ui/os/wm.hpp"

namespace izan::os {

// The dock: a frosted shelf of app marks with hover magnification,
// name tooltips and running dots. Clicks launch, restore or focus.
// Painted above every window; its shelf rectangle is handed to the
// window manager so window input never fires underneath it.
class Dock {
public:
    void frame(Wm& wm, const std::vector<App*>& apps, ImVec2 ws_min,
        ImVec2 ws_max);

    ImVec2 shelf_min() const
    {
        return shelf_min_;
    }

    ImVec2 shelf_max() const
    {
        return shelf_max_;
    }

private:
    ImVec2 shelf_min_ {};
    ImVec2 shelf_max_ {};
};

}
