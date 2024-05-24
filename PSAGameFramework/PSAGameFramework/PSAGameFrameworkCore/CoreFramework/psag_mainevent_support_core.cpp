// psag_mainevent_support_core.
#include "psag_mainevent_support.h"

using namespace std;
using namespace PSAG_LOGGER;

constexpr Vector4T<float> DefaultFrameColor = Vector4T<float>(0.0f, 0.16f, 0.16f, 0.92f);
constexpr const char*     EngineGLSL        = "#version 460 core";
constexpr const char*     EngineIconImage   = "PSAGameSystemFiles/PomeloStarGameLogo.png";

namespace PsagFrameworkCore {
    // opengl glfw version: 3.3 [20240428]
    uint32_t GLFWversionArray[2] = { 3,3 };

    // **************** core static init ****************

    GraphicsEnginePost::PsagGLEnginePost* PSAGame2DFramework::RendererPostFX             = nullptr;
    GraphicsEngineBackground::PsagGLEngineBackground* PSAGame2DFramework::RendererBackFX = nullptr;

    void PSAGame2DFramework::FrameworkRendererLowinit(const string& gl_version) {
        PsagLow::PsagSupGraphicsTool::PsagGraphicsSysinit GLINIT;
        // in_logger funcptr => init.
        GLINIT.LoggerFunc(PushLogger);
        GLINIT.RendererInit(FrameworkGraphicsParams, gl_version);
    }

    bool PSAGame2DFramework::FrameworkRenderingGameScene() {
        // rendering background-shader sys.
        if (RendererBackFX != nullptr)
            RendererBackFX->RenderingBackgroundModule();

        // run game_logic(game_scene).
        for (auto it = GAME_CORE_CLASS.begin(); it != GAME_CORE_CLASS.end(); ++it)
            it->second->LogicEventLoopGame(FrameworkParams);

        // update vertex_dataset state.
        SystemFrameUpdateNewState();
        return true;
    }

    bool PSAGame2DFramework::FrameworkRenderingGameGui() {
        // run gui_logic(game_gui).
        for (auto it = GAME_CORE_CLASS.begin(); it != GAME_CORE_CLASS.end(); ++it)
            it->second->LogicEventLoopGui(FrameworkParams);
        return true;
    }

    bool PSAGame2DFramework::CoreFrameworkEvent() {
        bool CoreErrorFlag = PSAG_FALSE;
        // state submit.
        FrameworkParams.PostShaderParams = &RendererPostFX->RenderParameters;
        FrameworkParams.BackShaderParams = &RendererBackFX->RenderParameters;
        FrameworkParams.WindowResolution = RenderingWinSize;

        // opengl render_event loop.
        RenderContextAbove();
        {
            if (RendererPostFX != nullptr) {
                CoreErrorFlag |= !RendererPostFX->CaptureGameScene([&]() { return FrameworkRenderingGameScene(); });
                // render_pipline out_render.
                CoreErrorFlag |= !RendererPostFX->RenderingPostModule();
            }
            RenderGuiContextA();
            CoreErrorFlag |= !FrameworkRenderingGameGui();
            RenderGuiContextB();
        }
        RenderContextBelow();

        // glfw close_window flag & renderer block error.
        return !(CloseFlag() || CoreErrorFlag);
    }

    bool PSAGame2DFramework::CoreFrameworkInit(FrameworkSysVersion version) {
        bool CoreInitErrorFlag = PSAG_FALSE;
        // init_config_func => init...
        FrameworkInitConfig(EngineGLSL);
        RenderingWinSize = Vector2T<uint32_t>(WindowInitConfig.WindowSizeWidth, WindowInitConfig.WindowSizeHeight);

        PushLogger(LogTrace, PSAGM_FRAME_CORE_LABEL, "core framework init,config...");
        // init => create => set_icon => vsync => sys_callback.
        CoreInitErrorFlag |= !GLFWwindowInit(GLFWversionArray, RendererMSAA, PSAG_FALSE, RendererWindowFixed);
        CoreInitErrorFlag |= !GLFWwindowCreate(WindowInitConfig);

        GLFWwindowSetIcon(EngineIconImage);

        CoreInitErrorFlag |= !GLFWwindowVsync(PSAG_TRUE);
        CoreInitErrorFlag |= !GLFWwindowCallback();

        PsagLow::PsagSupGraphicsTool::GraphicsINFO GLINFO;
        GLINFO.LoggerPrintInformation = true;

        // init_window => init_renderer_low(opengl).
        FrameworkRendererLowinit(EngineGLSL);

        GLINFO.InfoPrintPlatformParameters();
        GLINFO.InfoGetGPUmemoryCapacity();
        GLINFO.InfoGetVertexUnitsMax();
        GLINFO.InfoGetTextureUnitsMax();

        // init create(alloc) OGL[LLRES], ver_system(s). 
        GraphicsEngineDataset::GLEngineStcVertexData::LowLevelResourceCreate(PushLogger, 30);

        VertexDataObjectCreate();
        StaticVertexDataObjectCreate();
        VirtualTextureDataObjectCreate(Vector2T<uint32_t>(2048, 2048));

        // create game2d post-shader.
        RendererPostFX = new GraphicsEnginePost::PsagGLEnginePost(RenderingWinSize);

        // init imgui_core system.
        ImGuiInit(MainWindowObject, ImGuiInitConfig);

        // registration dev_class, objects.
        InitializeRegistrationDev();
        // imit gui_logic(scene,gui).
        for (auto it = GAME_CORE_CLASS.begin(); it != GAME_CORE_CLASS.end(); ++it)
            it->second->LogicInitialization();

        *RenderingFrameColorPtr = DefaultFrameColor;
        PushLogger(LogTrace, PSAGM_FRAME_CORE_LABEL, "pomelo_star game2d version: %.8lf", version);
        // return init_items sum_flag.
        return !CoreInitErrorFlag;
    }

    bool PSAGame2DFramework::CoreFrameworkCloseFree() {
        bool CoreInitErrorFlag = PSAG_FALSE;
        // free framework oper...

        if (RendererBackFX != nullptr) delete RendererBackFX;
        if (RendererPostFX != nullptr) delete RendererPostFX;

        VertexDataObjectDelete();
        StaticVertexDataObjectDelete();
        VirtualTextureDataObjectDelete();

        ImGuiFree();
        CoreInitErrorFlag |= !GraphicsEngineDataset::GLEngineStcVertexData::LowLevelResourceFree();
        CoreInitErrorFlag |= !GLFWwindowFree();
        return CoreInitErrorFlag;
    }

    // ******************************** RegisterSystem ********************************

    void PSAGame2DFramework::GameRegistrationObject(
        const char* class_name, const char* unique_name, GameLogic::INTERFACE_DEVCLASS_GAME* rclass
    ) {
        auto it = GAME_CORE_CLASS.find(unique_name);
        if (it != GAME_CORE_CLASS.end())
            PushLogger(LogWarning, PSAGM_FRAME_CORE_LABEL, "registration ref_object repeat_name.");
        else {
            if (rclass == nullptr) {
                PushLogger(LogError, PSAGM_FRAME_CORE_LABEL, "registration ref_object nullptr.");
                return;
            }
            GAME_CORE_CLASS[unique_name] = rclass;
            PushLogger(LogTrace, PSAGM_FRAME_CORE_LABEL, "registration ref_object name: %s", unique_name);
        }
    }

    void PSAGame2DFramework::GameDeleteObject(const char* unique_name) {
        auto ObjectPtrTemp = GameFindRegObject(unique_name);
        if (ObjectPtrTemp == nullptr) {
            PushLogger(LogError, PSAGM_FRAME_CORE_LABEL, "delete ref_object nullptr.");
            return;
        }
        delete ObjectPtrTemp;
        // delete map_item.
        GAME_CORE_CLASS.erase(unique_name);
        PushLogger(LogTrace, PSAGM_FRAME_CORE_LABEL, "delete ref_object name: %s", unique_name);
    }

    GameLogic::INTERFACE_DEVCLASS_GAME* PSAGame2DFramework::GameFindRegObject(const char* unique_name) {
        return (GAME_CORE_CLASS.find(unique_name) != GAME_CORE_CLASS.end()) ? (GAME_CORE_CLASS[unique_name]) : nullptr;
    }
}