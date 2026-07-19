cmake_minimum_required(VERSION 3.28)
project(izan-ui LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
    # -O0 miscompiles libstdc++ type_info comparisons under MinGW; an
    # optimized default is a load-bearing wall, not a preference.
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

# The imgui stack: glfw + freetype + Dear ImGui (docking), pinned
# tarballs, one static library.
include(cmake/gui_deps.cmake)

# SecureBytes for password fields arrives through izan-crypto; only
# the interface target (and sodium beneath it) actually builds.
FetchContent_Declare(izan_crypto
    GIT_REPOSITORY https://github.com/izandotai/izan-crypto.git
    GIT_TAG 83899cb
    EXCLUDE_FROM_ALL
)
# Reflection JSON for the demo shell's settings.
FetchContent_Declare(glaze
    URL https://github.com/stephenberry/glaze/archive/refs/tags/v7.8.4.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP ON
    EXCLUDE_FROM_ALL
)
# QR codes: nayuki's reference implementation, a single translation
# unit compiled into the kit.
FetchContent_Declare(qrcodegen
    URL https://github.com/nayuki/QR-Code-generator/archive/refs/tags/v1.8.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP ON
    SOURCE_SUBDIR cmake-subdir-does-not-exist
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(izan_crypto glaze qrcodegen)

# The window shell: borderless chrome, themes, font cascade, IME,
# console re-plumbing, layout guardianship.
add_library(izan_ui_shell STATIC
    ui/shell/app.cpp
    ui/shell/chrome_widgets.cpp
    ui/shell/fonts.cpp
    ui/shell/ime.cpp
    ui/shell/theme.cpp
    ui/shell/ui_layout.cpp
    ui/shell/win_chrome.cpp
    ui/shell/win_console.cpp
    ui/shell/win_cursors.cpp
)
add_library(izan::ui_shell ALIAS izan_ui_shell)
target_include_directories(izan_ui_shell PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(izan_ui_shell PUBLIC izan_imgui dwmapi imm32)

# The kit: every component drawn in the active design language.
add_library(izan_kit STATIC
    ${qrcodegen_SOURCE_DIR}/cpp/qrcodegen.cpp
    ui/widgets/address_field.cpp
    ui/widgets/amount_field.cpp
    ui/widgets/asset_row.cpp
    ui/widgets/avatar.cpp
    ui/widgets/button.cpp
    ui/widgets/card.cpp
    ui/widgets/choice_row.cpp
    ui/widgets/copy_text.cpp
    ui/widgets/design.cpp
    ui/widgets/dialog.cpp
    ui/widgets/empty_state.cpp
    ui/widgets/hyperlink.cpp
    ui/widgets/identity.cpp
    ui/widgets/label.cpp
    ui/widgets/list_row.cpp
    ui/widgets/menu.cpp
    ui/widgets/pill.cpp
    ui/widgets/qr.cpp
    ui/widgets/result_mark.cpp
    ui/widgets/search_field.cpp
    ui/widgets/select.cpp
    ui/widgets/spinner.cpp
    ui/widgets/step_dots.cpp
    ui/widgets/table.cpp
    ui/widgets/text_field.cpp
    ui/widgets/toggle.cpp
    ui/widgets/tooltip.cpp
    ui/widgets/tx_row.cpp
)
add_library(izan::kit ALIAS izan_kit)
target_include_directories(izan_kit PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE ${qrcodegen_SOURCE_DIR}/cpp)
# shell32: kit_hyperlink hands URLs to the system browser.
target_link_libraries(izan_kit PUBLIC izan_imgui izan_secure shell32)

# The i18n engine: an English base table with per-language overlays;
# headless, imgui-free, testable anywhere. Language TABLES belong to
# the application — this is only the machinery.
add_library(izan_i18n STATIC
    ui/i18n/catalog.cpp
)
add_library(izan::i18n ALIAS izan_i18n)
target_include_directories(izan_i18n PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(izan_i18n PRIVATE glaze::glaze)

# Copy the framework's runtime assets (fonts) beside a consumer's
# executable; applications layer their own assets on top.
function(izan_ui_copy_assets target)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${izan-ui_SOURCE_DIR}/ui/assets
            $<TARGET_FILE_DIR:${target}>/assets
        COMMENT "Copying izan-ui assets for ${target}")
endfunction()

if(PROJECT_IS_TOP_LEVEL)
    # The acceptance shell: full window chrome plus a component
    # gallery, double-click friendly (windows subsystem; a terminal
    # launch re-plumbs stdout back).
    add_executable(izan_ui_smoke WIN32 demo/smoke_main.cpp)
    target_link_libraries(
        izan_ui_smoke PRIVATE izan_ui_shell izan_kit glaze::glaze)
    izan_ui_copy_assets(izan_ui_smoke)
endif()
