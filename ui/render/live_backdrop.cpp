// The living wallpaper — sdf_rect's callback machinery with an aurora
// where the rounded rectangle used to be. Three sine-displaced bands
// breathe over a deep mint gradient, with one soft glow; everything is
// a few dozen ALU ops per pixel, no texture fetches.
#include "ui/render/live_backdrop.hpp"

#include <cstdint>
#include <vector>

#include <GLFW/glfw3.h>

namespace izan::render {

namespace {

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
          "uniform vec4 uQuad;\n"
          "void main(){\n"
          "  vec2 p = vec2(gl_VertexID==1||gl_VertexID==3 ? uQuad.z : "
          "uQuad.x,\n"
          "               gl_VertexID>=2 ? uQuad.w : uQuad.y);\n"
          "  gl_Position = vec4(p*2.0/uScreen*vec2(1,-1)+vec2(-1,1), 0, 1);\n"
          "}\n";

    constexpr const char* kFragment
        = "#version 130\n"
          "uniform vec2 uScreen;\n"
          "uniform vec4 uQuad;\n"
          "uniform float uTime;\n"
          "void main(){\n"
          "  vec2 pix = vec2(gl_FragCoord.x, uScreen.y - gl_FragCoord.y);\n"
          "  vec2 uv = (pix - uQuad.xy) / (uQuad.zw - uQuad.xy);\n"
          "  vec3 col = mix(vec3(0.094,0.220,0.188), vec3(0.039,0.110,0.082),"
          "               uv.y);\n"
          "  float t = uTime*0.06;\n"
          "  for (int i = 0; i < 3; i++){\n"
          "    float fi = float(i);\n"
          "    float y = uv.y - (0.30 + 0.20*fi);\n"
          "    float w = sin(uv.x*(3.0+fi*1.7) + t*(1.0+0.6*fi) + fi*2.1)"
          "*0.06\n"
          "            + sin(uv.x*(7.0+fi*2.3) - t*(1.3+0.4*fi))*0.025;\n"
          "    float band = exp(-pow((y - w)*(9.0 - fi*2.0), 2.0));\n"
          "    vec3 tint = mix(vec3(0.33,0.72,0.42), vec3(0.16,0.52,0.58),"
          "                   fi*0.5);\n"
          "    col += tint * band * (0.11 + 0.05*sin(t*0.7 + fi*1.9));\n"
          "  }\n"
          "  float g = exp(-length((uv - vec2(0.76,0.24))*vec2(1.6,2.2))"
          "*1.8);\n"
          "  col += vec3(0.30,0.55,0.30)*g*0.22;\n"
          "  gl_FragColor = vec4(col, 1.0);\n"
          "}\n";

    struct Program {
        GLu id = 0;
        GLi u_screen = -1, u_quad = -1, u_time = -1;
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
            v.u_time = gl().GetUniformLocation(v.id, "uTime");
            return v;
        }();
        return p;
    }

    struct Params {
        ImVec2 min, max;
        float time;
    };

    std::vector<Params> g_arena;
    int g_arena_frame = -1;

    void render_callback(const ImDrawList*, const ImDrawCmd* cmd)
    {
        const Program& prog = program();
        if (prog.failed)
            return;
        const std::size_t index = static_cast<std::size_t>(
            reinterpret_cast<intptr_t>(cmd->UserCallbackData));
        if (index >= g_arena.size())
            return;
        const Params& p = g_arena[index];
        const ImGuiViewport* vp = ImGui::GetMainViewport();

        gl().UseProgram(prog.id);
        gl().Uniform2f(prog.u_screen, vp->Size.x, vp->Size.y);
        gl().Uniform4f(prog.u_quad, p.min.x, p.min.y, p.max.x, p.max.y);
        gl().Uniform1f(prog.u_time, p.time);
        gl().Scissor(GLi(cmd->ClipRect.x), GLi(vp->Size.y - cmd->ClipRect.w),
            GLi(cmd->ClipRect.z - cmd->ClipRect.x),
            GLi(cmd->ClipRect.w - cmd->ClipRect.y));
        constexpr GLu kTriangleStrip = 0x0005;
        gl().DrawArrays(kTriangleStrip, 0, 4);
    }

}

bool live_backdrop(ImDrawList* draw, ImVec2 min, ImVec2 max, float time_s)
{
    if (program().failed)
        return false;
    const int frame = ImGui::GetFrameCount();
    if (frame != g_arena_frame) {
        g_arena.clear();
        g_arena_frame = frame;
    }
    const std::size_t index = g_arena.size();
    g_arena.push_back({ min, max, time_s });
    draw->AddCallback(
        render_callback, reinterpret_cast<void*>(static_cast<intptr_t>(index)));
    draw->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
    return true;
}

}
