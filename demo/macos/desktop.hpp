#pragma once

#include <imgui.h>

#include "state.hpp"

namespace macos {

// The composition root: owns the aggregate state and stacks the
// components in paint order inside whatever rectangle the host
// hands over.
class Desktop {
public:
    void render(ImVec2 pos, ImVec2 size);

private:
    DesktopState state_;
};

}
