#include "ui/render/sdf_rect.hpp"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <vector>

#include <GLFW/glfw3.h>

namespace izan::render {

namespace {

    // ---- the minimal GL surface this module touches ----
    typedef unsigned GLu;
    typedef int GLi;
    typedef GLu(APIENTRY* CreateShaderFn)(GLu);
    typedef void(APIENTRY* ShaderSourceFn)(
        GLu, GLi, const char* const*, const GLi*);
    typedef void(APIENTRY* CompileShaderFn)(GLu);
    typedef void(APIENTRY* GetShaderivFn)(GLu, GLu, GLi*);
    typedef GLu(APIENTRY* CreateProgramFn)();
    typedef void(APIENTRY* AttachShaderFn)(GLu, GLu);
    typedef void(APIENTRY* LinkProgramFn)(GLu);
    typedef void(APIENTRY* GetProgramivFn)(GLu, GLu, GLi*);
    typedef void(APIENTRY* UseProgramFn)(GLu);
    typedef GLi(APIENTRY* GetUniformLocationFn)(GLu, const char*);
    typedef void(APIENTRY* Uniform4fFn)(GLi, float, float, float, float);
    typedef void(APIENTRY* Uniform2fFn)(GLi, float, float);
    typedef void(APIENTRY* Uniform1fFn)(GLi, float);
    typedef void(APIENTRY* DrawArraysFn)(GLu, GLi, GLi);
    typedef void(APIENTRY* ScissorFn)(GLi, GLi, GLi, GLi);

    struct Gl {
        CreateShaderFn CreateShader = nullptr;
        ShaderSourceFn ShaderSource = nullptr;
        CompileShaderFn CompileShader = nullptr;
        GetShaderivFn GetShaderiv = nullptr;
        CreateProgramFn CreateProgram = nullptr;
        AttachShaderFn AttachShader = nullptr;
        LinkProgramFn LinkProgram = nullptr;
        GetProgramivFn GetProgramiv = nullptr;
        UseProgramFn UseProgram = nullptr;
        GetUniformLocationFn GetUniformLocation = nullptr;
        Uniform4fFn Uniform4f = nullptr;
        Uniform2fFn Uniform2f = nullptr;
        Uniform1fFn Uniform1f = nullptr;
        DrawArraysFn DrawArrays = nullptr;
        ScissorFn Scissor = nullptr;
        bool ok = false;
    };

    Gl& gl()
    {
        static Gl g = [] {
            Gl v;
            v.CreateShader = reinterpret_cast<CreateShaderFn>(
                glfwGetProcAddress("glCreateShader"));
            v.ShaderSource = reinterpret_cast<ShaderSourceFn>(
                glfwGetProcAddress("glShaderSource"));
            v.CompileShader = reinterpret_cast<CompileShaderFn>(
                glfwGetProcAddress("glCompileShader"));
            v.GetShaderiv = reinterpret_cast<GetShaderivFn>(
                glfwGetProcAddress("glGetShaderiv"));
            v.CreateProgram = reinterpret_cast<CreateProgramFn>(
                glfwGetProcAddress("glCreateProgram"));
            v.AttachShader = reinterpret_cast<AttachShaderFn>(
                glfwGetProcAddress("glAttachShader"));
            v.LinkProgram = reinterpret_cast<LinkProgramFn>(
                glfwGetProcAddress("glLinkProgram"));
            v.GetProgramiv = reinterpret_cast<GetProgramivFn>(
                glfwGetProcAddress("glGetProgramiv"));
            v.UseProgram = reinterpret_cast<UseProgramFn>(
                glfwGetProcAddress("glUseProgram"));
            v.GetUniformLocation = reinterpret_cast<GetUniformLocationFn>(
                glfwGetProcAddress("glGetUniformLocation"));
            v.Uniform4f = reinterpret_cast<Uniform4fFn>(
                glfwGetProcAddress("glUniform4f"));
            v.Uniform2f = reinterpret_cast<Uniform2fFn>(
                glfwGetProcAddress("glUniform2f"));
            v.Uniform1f = reinterpret_cast<Uniform1fFn>(
                glfwGetProcAddress("glUniform1f"));
            v.DrawArrays = reinterpret_cast<DrawArraysFn>(
                glfwGetProcAddress("glDrawArrays"));
            v.Scissor
                = reinterpret_cast<ScissorFn>(glfwGetProcAddress("glScissor"));
            v.ok = v.CreateShader && v.ShaderSource && v.CompileShader
                && v.GetShaderiv && v.CreateProgram && v.AttachShader
                && v.LinkProgram && v.GetProgramiv && v.UseProgram
                && v.GetUniformLocation && v.Uniform4f && v.Uniform2f
                && v.Uniform1f && v.DrawArrays && v.Scissor;
            return v;
        }();
        return g;
    }

    constexpr const char* kVertex
        = "#version 130\n"
          "uniform vec2 uScreen;\n"
          "uniform vec4 uQuad;\n" // min.xy max.xy, screen px, y-down
          "void main(){\n"
          "  vec2 p = vec2(gl_VertexID==1||gl_VertexID==3 ? uQuad.z : "
          "uQuad.x,\n"
          "               gl_VertexID>=2 ? uQuad.w : uQuad.y);\n"
          "  gl_Position = vec4(p*2.0/uScreen*vec2(1,-1)+vec2(-1,1), 0, 1);\n"
          "}\n";

    constexpr const char* kFragment
        = "#version 130\n"
          "uniform vec2 uScreen;\n"
          "uniform vec4 uRect;\n"
          "uniform vec4 uRadii;\n" // tl tr br bl
          "uniform vec4 uFill;\n"
          "uniform vec4 uBorder;\n"
          "uniform float uBorderPx;\n"
          "uniform float uSoft;\n"
          "float sd(vec2 p, vec2 b, float r){\n"
          "  vec2 q = abs(p) - b + r;\n"
          "  return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - r;\n"
          "}\n"
          "void main(){\n"
          "  vec2 pix = vec2(gl_FragCoord.x, uScreen.y - gl_FragCoord.y);\n"
          "  vec2 c = (uRect.xy + uRect.zw) * 0.5;\n"
          "  vec2 half_size = (uRect.zw - uRect.xy) * 0.5;\n"
          "  vec2 p = pix - c;\n"
          "  float r = p.y < 0.0 ? (p.x < 0.0 ? uRadii.x : uRadii.y)\n"
          "                      : (p.x < 0.0 ? uRadii.w : uRadii.z);\n"
          "  float d = sd(p, half_size, r);\n"
          "  if (uSoft > 0.0) {\n"
          "    float a = d <= 0.0 ? 1.0 : exp(-(d*d)/(2.0*uSoft*uSoft));\n"
          "    if (uFill.a * a <= 0.003) discard;\n"
          "    gl_FragColor = vec4(uFill.rgb, uFill.a * a);\n"
          "    return;\n"
          "  }\n"
          "  float aa = 0.65;\n"
          "  float outer = 1.0 - smoothstep(-aa, aa, d);\n"
          "  float inner = 1.0 - smoothstep(-aa, aa, d + uBorderPx);\n"
          "  vec4 col = uBorderPx > 0.0 ? mix(uBorder, uFill, inner) : uFill;\n"
          "  if (outer * col.a <= 0.003) discard;\n"
          "  gl_FragColor = vec4(col.rgb, col.a * outer);\n"
          "}\n";

    struct Program {
        GLu id = 0;
        GLi u_screen = -1, u_quad = -1, u_rect = -1, u_radii = -1;
        GLi u_fill = -1, u_border = -1, u_border_px = -1, u_soft = -1;
        bool failed = false;
    };

    Program& program()
    {
        static Program p = [] {
            Program v;
            if (!gl().ok) {
                v.failed = true;
                return v;
            }
            constexpr GLu kVert = 0x8B31, kFrag = 0x8B30;
            constexpr GLu kCompileStatus = 0x8B81, kLinkStatus = 0x8B82;
            const auto compile = [](GLu type, const char* src) -> GLu {
                const GLu s = gl().CreateShader(type);
                gl().ShaderSource(s, 1, &src, nullptr);
                gl().CompileShader(s);
                GLi ok = 0;
                gl().GetShaderiv(s, kCompileStatus, &ok);
                return ok != 0 ? s : 0;
            };
            const GLu vs = compile(kVert, kVertex);
            const GLu fs = compile(kFrag, kFragment);
            if (vs == 0 || fs == 0) {
                v.failed = true;
                return v;
            }
            v.id = gl().CreateProgram();
            gl().AttachShader(v.id, vs);
            gl().AttachShader(v.id, fs);
            gl().LinkProgram(v.id);
            GLi ok = 0;
            gl().GetProgramiv(v.id, kLinkStatus, &ok);
            if (ok == 0) {
                v.failed = true;
                return v;
            }
            v.u_screen = gl().GetUniformLocation(v.id, "uScreen");
            v.u_quad = gl().GetUniformLocation(v.id, "uQuad");
            v.u_rect = gl().GetUniformLocation(v.id, "uRect");
            v.u_radii = gl().GetUniformLocation(v.id, "uRadii");
            v.u_fill = gl().GetUniformLocation(v.id, "uFill");
            v.u_border = gl().GetUniformLocation(v.id, "uBorder");
            v.u_border_px = gl().GetUniformLocation(v.id, "uBorderPx");
            v.u_soft = gl().GetUniformLocation(v.id, "uSoft");
            return v;
        }();
        return p;
    }

    // Per-frame parameter arena; callbacks carry an index into it.
    std::vector<SdfRect> g_arena;
    int g_arena_frame = -1;

    void set_color(GLi loc, ImU32 c)
    {
        gl().Uniform4f(loc, float((c >> IM_COL32_R_SHIFT) & 0xff) / 255.0f,
            float((c >> IM_COL32_G_SHIFT) & 0xff) / 255.0f,
            float((c >> IM_COL32_B_SHIFT) & 0xff) / 255.0f,
            float((c >> IM_COL32_A_SHIFT) & 0xff) / 255.0f);
    }

    void render_callback(const ImDrawList*, const ImDrawCmd* cmd)
    {
        const Program& prog = program();
        if (prog.failed)
            return;
        const std::size_t index = static_cast<std::size_t>(
            reinterpret_cast<intptr_t>(cmd->UserCallbackData));
        if (index >= g_arena.size())
            return;
        const SdfRect& r = g_arena[index];
        const ImGuiViewport* vp = ImGui::GetMainViewport();

        gl().UseProgram(prog.id);
        gl().Uniform2f(prog.u_screen, vp->Size.x, vp->Size.y);
        // The quad covers the rect plus the AA fringe — or the whole
        // gaussian tail when the rect is a soft shadow.
        const float fringe = r.soft_px > 0.0f ? r.soft_px * 3.0f : 2.0f;
        gl().Uniform4f(prog.u_quad, r.min.x - fringe, r.min.y - fringe,
            r.max.x + fringe, r.max.y + fringe);
        gl().Uniform4f(prog.u_rect, r.min.x, r.min.y, r.max.x, r.max.y);
        gl().Uniform4f(
            prog.u_radii, r.radius[0], r.radius[1], r.radius[2], r.radius[3]);
        set_color(prog.u_fill, r.fill);
        set_color(prog.u_border, r.border);
        gl().Uniform1f(prog.u_border_px, r.border_px);
        gl().Uniform1f(prog.u_soft, r.soft_px);
        // Honor the clip the draw list carried for this command.
        gl().Scissor(GLi(cmd->ClipRect.x), GLi(vp->Size.y - cmd->ClipRect.w),
            GLi(cmd->ClipRect.z - cmd->ClipRect.x),
            GLi(cmd->ClipRect.w - cmd->ClipRect.y));
        constexpr GLu kTriangleStrip = 0x0005;
        gl().DrawArrays(kTriangleStrip, 0, 4);
    }

}

void sdf_rect(ImDrawList* draw, const SdfRect& rect)
{
    // IZAN_SDF_OFF=1 forces the polygon fallback - the A/B switch
    // for isolating shader-path side effects.
    static const bool forced_off = std::getenv("IZAN_SDF_OFF") != nullptr;
    if (forced_off || program().failed) {
        // The polygon path is the honest fallback: one radius, the
        // largest requested, and the classic stroked rim. A soft
        // shadow degrades to a single translucent slab.
        const float r = std::max(std::max(rect.radius[0], rect.radius[1]),
            std::max(rect.radius[2], rect.radius[3]));
        if (rect.soft_px > 0.0f) {
            ImU32 half = rect.fill;
            half = (half & ~IM_COL32_A_MASK)
                | ((((half >> IM_COL32_A_SHIFT) & 0xff) / 2)
                    << IM_COL32_A_SHIFT);
            draw->AddRectFilled(
                ImVec2(rect.min.x - rect.soft_px, rect.min.y - rect.soft_px),
                ImVec2(rect.max.x + rect.soft_px, rect.max.y + rect.soft_px),
                half, r + rect.soft_px);
            return;
        }
        draw->AddRectFilled(rect.min, rect.max, rect.fill, r);
        if (rect.border_px > 0.0f)
            draw->AddRect(
                rect.min, rect.max, rect.border, r, 0, rect.border_px);
        return;
    }
    const int frame = ImGui::GetFrameCount();
    if (frame != g_arena_frame) {
        g_arena.clear();
        g_arena_frame = frame;
    }
    const std::size_t index = g_arena.size();
    g_arena.push_back(rect);
    draw->AddCallback(
        render_callback, reinterpret_cast<void*>(static_cast<intptr_t>(index)));
    draw->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
}

}
