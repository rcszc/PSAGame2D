// framework_window_system.
#include "framework_window.h"

using namespace std;
using namespace PSAG_LOGGER;

#ifdef DEVICE_ENABLE_NVIDIA_GPU
// USE NVIDIA DEVICE GPU.
extern "C" { 
    _declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001; 
}
#endif
namespace system_err {
    // glfw system callback.
    static void GLFWsys_ErrorCallback(int error, const char* description) {
        // callback static_func: print: GLFW_ERROR msg.
        PushLogger(LogError, "[GLFW]", "library error %d: %s", error, description);
    }

    inline bool ModuleLogger(GLenum glerr, const char* modulename, const char* message) {
        if (glerr != NULL) {
            // glerror => print logsystem.
            PushLogger(LogError, PSAG_WINDOW_OGLFW::PSAG_WINDOW_LABEL, "fw_window %s code: %u", modulename, glerr);
            PushLogger(LogError, PSAG_WINDOW_OGLFW::PSAG_WINDOW_LABEL, "fw_window %s", message);
            return false;
        }
        return true;
    }
}

#define GLFW_VERSION_HEADER 3
namespace PSAG_WINDOW_OGLFW {
    inline void InitSystemWindowClear(GLFWwindow* window) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        // init clear window.
        glfwMakeContextCurrent(window);
        glfwSwapBuffers(window);
    }
#define PRESET_SCROLL_POS 25.0f
#define PRESET_SCROLL_MIN 0.0f
#define PRESET_SCROLL_MAX 50.0f

    Vector3T<float> PsagGLFWsystemCALLBACK::ValueMouseScroll = Vector3T<float>(PRESET_SCROLL_POS, PRESET_SCROLL_MIN, PRESET_SCROLL_MAX);
    Vector2T<float> PsagGLFWsystemCALLBACK::ValueMouseCursor = {};
    bool            PsagGLFWsystemCALLBACK::ValueWindowFocus = false;
    vector<string>  PsagGLFWsystemCALLBACK::FilePaths        = {};
    Vector2T<float> PsagGLFWsystemCALLBACK::ValueWindowSize  = Vector2T<float>(1.0f, 1.0f);

    /*
    * 获取信息:
    * "GLFWsystemCallback::CallbackScroll"     鼠标滚轮事件回调[FW]
    * "GLFWsystemCallback::CallbackCursorPos"  鼠标位置回调[FW]
    * "GLFWsystemCallback::CallbackFocus"      窗口聚焦标志回调[FW]
    * "GLFWsystemCallback::CallbackDropFiles"  向窗口内拖拽文件回调[FW]
    * "GLFWsystemCallback::CallbackWindowSize" 窗口大小回调[FW]
    */

    void PsagGLFWsystemCALLBACK::CallbackScroll(GLFWwindow* window, double xoffset, double yoffset) {
        // calc scroll x:pos, y:min, z:max.
        if (ValueMouseScroll.vector_x >= ValueMouseScroll.vector_y && (ValueMouseScroll.vector_x <= ValueMouseScroll.vector_z)) {
            ValueMouseScroll.vector_x -= float(yoffset) * 2.0f;
        }
        ValueMouseScroll.vector_x = ValueMouseScroll.vector_x <= ValueMouseScroll.vector_y ? ValueMouseScroll.vector_y : ValueMouseScroll.vector_x;
        ValueMouseScroll.vector_x = ValueMouseScroll.vector_x >= ValueMouseScroll.vector_z ? ValueMouseScroll.vector_z : ValueMouseScroll.vector_x;
    }

    void PsagGLFWsystemCALLBACK::CallbackCursorPos(GLFWwindow* window, double xpos, double ypos) {
        ValueMouseCursor.vector_x = (float)xpos;
        ValueMouseCursor.vector_y = (float)ypos;
    }

    void PsagGLFWsystemCALLBACK::CallbackFocus(GLFWwindow* window, int focus) {
        // glfw system-window focus.
        ValueWindowFocus = (bool)focus;
    }

    void PsagGLFWsystemCALLBACK::CallbackDropFiles(GLFWwindow* window, int count, const char** paths) {
        for (size_t i = 0; i < count; ++i)
            FilePaths.push_back(string(paths[i]));
    }

    void PsagGLFWsystemCALLBACK::CallbackWindowSize(GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);
        ValueWindowSize.vector_x = (float)width;
        ValueWindowSize.vector_y = (float)height;
    }

    // 窗口关闭(键),回调[FW]
    void PsagGLFWsystemCALLBACK::CallbackWindowClose(GLFWwindow* window) {}

    std::function<void(std::chrono::steady_clock::time_point)> PsagGLFWsystemCALLBACK::REFRESH_CALL_FUNC = 
        [&](std::chrono::steady_clock::time_point) {};
    void PsagGLFWsystemCALLBACK::CallbackWindowRefresh(GLFWwindow* window) {
        REFRESH_CALL_FUNC(chrono::steady_clock::now());
    }

#ifdef DEVICE_ENABLE_WINAPI_PROCESS
    WNDPROC PsagGLFWsystemCallback::OLD_WINDOW_PROC = nullptr;
    HWND    PsagGLFWsystemCallback::OLD_WINDOW_HWND = nullptr;
#endif

    bool PasgWindowEvent::GLFWwindowCreate(FwWindowConfig config) {
        // glfw system error-callback.
        glfwSetErrorCallback(system_err::GLFWsys_ErrorCallback);

        GLFWmonitor* Monitor = {};

        if (config.WindowFullFlag) {
            // 创建全屏无框窗口.
            Monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* Mode = glfwGetVideoMode(Monitor);
            MainWindowObject = glfwCreateWindow(Mode->width, Mode->height, config.WindowName.c_str(), Monitor, NULL);
            ValueWindowSize.vector_x = (float)Mode->width;
            ValueWindowSize.vector_y = (float)Mode->height;
        }
        else {
            // create window object.
            MainWindowObject = glfwCreateWindow(
                config.WindowSizeWidth, config.WindowSizeHeight, config.WindowName.c_str(),
                Monitor, nullptr
            );
            ValueWindowSize.vector_x = (float)config.WindowSizeWidth;
            ValueWindowSize.vector_y = (float)config.WindowSizeHeight;
        }

        if (MainWindowObject == nullptr) {
            PushLogger(LogError, PSAG_WINDOW_LABEL, "failed fw_window create object.");
            return false;
        }
        glfwMakeContextCurrent(MainWindowObject);
        // init draw window buffer.
        InitSystemWindowClear(MainWindowObject);
        return true;
    }

    bool PasgWindowEvent::GLFWwindowInit(uint32_t version[2], uint32_t MSAA, bool core_mode, bool fixedsize) {
        if (glfwInit() == NULL) {
            PushLogger(LogError, PSAG_WINDOW_LABEL, "failed fw_window init config.");
            return false;
        }
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, version[0]);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, version[1]);

        // COMPAT PROFILE 兼容模式.
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
#if (PSAG_DEBUG_MODE == false)
        // opengl_ext:"GL_KHR_no_error" => no_err_state. psag(release).
        glfwWindowHint(GLFW_CONTEXT_NO_ERROR, GLFW_TRUE);
#endif
        // glfw.version 3.2+ [profile].
        // glfw.version 3.0+
        if (version[0] > GLFW_VERSION_HEADER) {
            if (core_mode)
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        }
        glfwWindowHint(GLFW_DEPTH_BITS, 24);        // depth buffer.
        glfwWindowHint(GLFW_RESIZABLE, !fixedsize); // fixed window size.
        glfwWindowHint(GLFW_SAMPLES, MSAA);         // samples MSAA.

        // GLFW 窗口透明, 当前默认不采用. RCSZ 2023_12_26.
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);

        PushLogger(LogInfo, PSAG_WINDOW_LABEL, "init opengl_glfw.");
        // init opengl glfw completed => check get_error status.
        return system_err::ModuleLogger(glfwGetError(&ErrorMessage), "global_init", ErrorMessage);
    }

    // STB_IMAGE iamge_loader, not 'LLGraphics'.
#include "stb_image.h"
    void PasgWindowEvent::GLFWwindowSetIcon(const string& file) {
        int ImageChannels = NULL;

        GLFWimage IconImage = {};
        if (filesystem::exists(file)) {
            // load program icon image_texture file.
            IconImage.pixels = stbi_load(file.c_str(), &IconImage.width, &IconImage.height, &ImageChannels, NULL);
            glfwSetWindowIcon(MainWindowObject, 1, &IconImage);
            stbi_image_free(IconImage.pixels);
            PushLogger(LogInfo, PSAG_WINDOW_LABEL, "fw_window load: icon_image: %s", file.c_str());
        }
        else
            PushLogger(LogWarning, PSAG_WINDOW_LABEL, "failed fw_window load: icon_image.");
    }

    bool PasgWindowEvent::GLFWwindowVsync(bool vswitch) {
        // 垂直同步 (vertical synchronization).
        glfwSwapInterval((int)vswitch);
        // check glfw vsync enable info_error.
        return system_err::ModuleLogger(
            glfwGetError(&ErrorMessage), "render_vsync", ErrorMessage
        );
    }

    constexpr const char* CALLBACK_MSG[7] = {
        "callback:'scroll'", "callback:'cursor position'", "callback:'window focus'", "callback:'drop files'",
        "callback:'window resize'", "callback:'window close'", "callback:'window refresh'"
    };
    bool PasgWindowEvent::GLFWwindowCallback() {
        bool ReturnFlag = true;

        glfwSetScrollCallback(MainWindowObject, CallbackScroll);
        ReturnFlag = system_err::ModuleLogger(glfwGetError(&ErrorMessage), CALLBACK_MSG[0], ErrorMessage);

        glfwSetCursorPosCallback(MainWindowObject, CallbackCursorPos);
        ReturnFlag = system_err::ModuleLogger(glfwGetError(&ErrorMessage), CALLBACK_MSG[1], ErrorMessage);

        glfwSetWindowFocusCallback(MainWindowObject, CallbackFocus);
        ReturnFlag = system_err::ModuleLogger(glfwGetError(&ErrorMessage), CALLBACK_MSG[2], ErrorMessage);

        glfwSetDropCallback(MainWindowObject, CallbackDropFiles);
        ReturnFlag = system_err::ModuleLogger(glfwGetError(&ErrorMessage), CALLBACK_MSG[3], ErrorMessage);

        glfwSetFramebufferSizeCallback(MainWindowObject, CallbackWindowSize);
        ReturnFlag = system_err::ModuleLogger(glfwGetError(&ErrorMessage), CALLBACK_MSG[4], ErrorMessage);

        glfwSetWindowCloseCallback(MainWindowObject, CallbackWindowClose);
        ReturnFlag = system_err::ModuleLogger(glfwGetError(&ErrorMessage), CALLBACK_MSG[5], ErrorMessage);

        glfwSetWindowRefreshCallback(MainWindowObject, CallbackWindowRefresh);
        ReturnFlag = system_err::ModuleLogger(glfwGetError(&ErrorMessage), CALLBACK_MSG[6], ErrorMessage);

#ifdef DEVICE_ENABLE_WINAPI_PROCESS
        OLD_WINDOW_HWND = glfwGetWin32Window(MainWindowObject);
        //OLD_WINDOW_PROC = (WNDPROC);
#endif
        return ReturnFlag;
    }

    bool PasgWindowEvent::GLFWwindowFree() {
        // free window object.
        glfwDestroyWindow(MainWindowObject);
        glfwTerminate();

        PushLogger(LogInfo, PSAG_WINDOW_LABEL, "fw_window free: glfw context.");
        return system_err::ModuleLogger(glfwGetError(&ErrorMessage), "free opengl_glfw", ErrorMessage);
    }

    void PasgWindowEvent::RenderContextAbove() {
        glfwPollEvents();
        
        glfwGetFramebufferSize(MainWindowObject, &RenderBufferSize.vector_x, &RenderBufferSize.vector_y);
        glViewport(NULL, NULL, RenderBufferSize.vector_x, RenderBufferSize.vector_y);

        glClearColor(RenderClearColor.vector_x, RenderClearColor.vector_y, RenderClearColor.vector_z, RenderClearColor.vector_w);
        glClearDepth(1.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void PasgWindowEvent::RenderContextBelow() {
        // set render context, swap buffer.
        glfwMakeContextCurrent(MainWindowObject);
        glfwSwapBuffers(MainWindowObject);
    }

    chrono::steady_clock::time_point PasgWindowEvent::FrameTimer = chrono::steady_clock::now();
    float PasgWindowEvent::CalcFrameTime       = 0.0f;
    float PasgWindowEvent::CalcStepTimeTemp[2] = { 1.0f,1.0f };

    constexpr float SAMPLER_TIME_MS = 500.0f;
    // value: 0.032f [20240528.1644], value: 0.045f [20240707.1749]
    constexpr float INTER_SPEED_VALUE = 0.045f;

    void PasgWindowEvent::ClacFrameTimeStepBegin() {
        // frame timer start(begin).
        FrameTimer = chrono::steady_clock::now();
    }

    float PasgWindowEvent::CalcFrameTimeStepEnd(float base_fps) {
        // calc frame_time: accuracy(us).
        CalcFrameTime = (float)chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - FrameTimer).count();
        CalcFrameTime /= 1000000.0f; // us convert sec.
        // calc base_fps step_scale.
        CalcStepTimeTemp[0] = base_fps / (1.0f / CalcFrameTime);
        CalcStepTimeTemp[1] += (CalcStepTimeTemp[0] - CalcStepTimeTemp[1]) * INTER_SPEED_VALUE;
        return CalcStepTimeTemp[1];
    }
}