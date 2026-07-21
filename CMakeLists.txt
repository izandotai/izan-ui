cmake_minimum_required(VERSION 3.28)
project(izan-ui LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
    # -O0 miscompiles libstdc++ type_info comparisons under MinGW; an
    # optimized default is a load-bearing wall, not a preference.
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

if(MINGW)
    # Fully static: an executable may import Windows system libraries
    # and NOTHING else — no compiler-runtime DLLs ride along. DEP and
    # ASLR link bits set explicitly; mingw does not guarantee them.
    add_link_options(-static)
    add_link_options(-Wl,--nxcompat -Wl,--dynamicbase -Wl,--high-entropy-va)
    add_compile_definitions(_WIN32_WINNT=0x0A00)
endif()

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

# OFF = i18n engine only: no imgui stack is even downloaded, so a
# consumer's trust-plane build stays lean.
option(IZAN_UI_SHELL "Build the imgui shell and widget kit" ON)

# SecureBytes for password fields arrives through izan-crypto; only
# the interface target (and sodium beneath it) actually builds.
# Consumers that also depend on izan-crypto or glaze must keep their
# pins in lockstep — the first declare wins.
FetchContent_Declare(izan_crypto
    GIT_REPOSITORY https://github.com/izandotai/izan-crypto.git
    GIT_TAG 6ced820
    EXCLUDE_FROM_ALL
)
# Reflection JSON for the i18n tables and the demo shell's settings.
FetchContent_Declare(glaze
    URL https://github.com/stephenberry/glaze/archive/refs/tags/v7.8.4.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP ON
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(izan_crypto glaze)

if(IZAN_UI_SHELL)
    # The imgui stack: glfw + freetype + Dear ImGui (docking), pinned
    # tarballs, one static library.
    include(cmake/gui_deps.cmake)
    # QR codes: nayuki's reference implementation, a single
    # translation unit compiled into the kit.
    FetchContent_Declare(qrcodegen
        URL https://github.com/nayuki/QR-Code-generator/archive/refs/tags/v1.8.0.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP ON
        SOURCE_SUBDIR cmake-subdir-does-not-exist
        EXCLUDE_FROM_ALL
    )
    FetchContent_MakeAvailable(qrcodegen)
endif()

if(IZAN_UI_SHELL)

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
    ui/widgets/scrollbar.cpp
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
target_link_libraries(
    izan_kit PUBLIC izan_imgui izan_secure shell32 PRIVATE izan_render)
endif()

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

# izan OS: the desktop-grade windowing kernel every family product
# stands on. Mechanism (window manager, panel, app registry) is
# style-free; every window pixel routes through the Theme interface —
# Mint-cultured is the working development skin, another look is a
# new implementation, the kernel untouched.
if(IZAN_UI_SHELL)
    # Analytic (SDF) drawing: shader-measured coverage for rounded
    # shapes — smooth at any zoom, cheaper than polygon fans. Plus the
    # SVG road: Noto emoji artwork rasterized through nanosvg.
    add_library(izan_render STATIC
        ui/render/image_load.cpp
        ui/render/svg_icon.cpp
        ui/render/sdf_rect.cpp
        ui/render/svg_raster.cpp
    )
    add_library(izan::render ALIAS izan_render)
    target_include_directories(izan_render PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    target_include_directories(izan_render PRIVATE
        ${nanosvg_SOURCE_DIR}/src ${stb_SOURCE_DIR})
    target_link_libraries(izan_render PUBLIC izan_imgui PRIVATE glfw)

    # Proof sheet: SVG icons rasterized at UI sizes to a BMP, no
    # window needed.
    add_executable(izan_svg_sheet tools/svg_sheet.cpp)
    target_link_libraries(izan_svg_sheet PRIVATE izan_render)

    add_library(izan_os STATIC
        ui/os/mint_paint.cpp
        ui/os/panel.cpp
        ui/os/shell.cpp
        ui/os/theme_mint.cpp
        ui/os/wm.cpp
    )
    add_library(izan::os ALIAS izan_os)
    target_include_directories(izan_os PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(izan_os PUBLIC izan_imgui PRIVATE izan_kit izan_render)
endif()

if(PROJECT_IS_TOP_LEVEL AND IZAN_UI_SHELL)
    # The acceptance shell: full window chrome plus a component
    # gallery, double-click friendly (windows subsystem; a terminal
    # launch re-plumbs stdout back).
    add_executable(izan_ui_smoke WIN32 demo/smoke_main.cpp)
    target_link_libraries(
        izan_ui_smoke PRIVATE izan_ui_shell izan_kit glaze::glaze)
    izan_ui_copy_assets(izan_ui_smoke)

    # The izan OS acceptance shell: kernel + working theme + three
    # apps of live kit widgets on the panel.
    add_executable(izan_ui_os_demo WIN32 demo/os/main.cpp)
    target_link_libraries(
        izan_ui_os_demo PRIVATE izan_ui_shell izan_os izan_kit)
    izan_ui_copy_assets(izan_ui_os_demo)
endif()
