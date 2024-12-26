// psag_mainevent_support_core.
#include "psag_mainevent_support.h"

using namespace std;
using namespace PSAG_LOGGER;

constexpr Vector4T<float> DefaultFrameColor = Vector4T<float>(0.0f, 0.0f, 0.0f, 1.0f);
constexpr const char*     EngineGLSL        = "#version 460 core";
constexpr const char*     EngineIconImage   = "PSAGameSystemFiles/PomeloStarGameLogo.png";

namespace PsagFrameworkCore {
    // opengl glfw version: 3.3 [20240428]
    uint32_t GLFWversionArray[2] = { 3,3 };

    // **************** core static init ****************

    GraphicsEngineFinal::PsagGLEngineFinal*               PSAGame2DFramework::RendererPostFX = nullptr;
    GraphicsEngineBackground::PsagGLEngineBackgroundBase* PSAGame2DFramework::RendererBackFX = nullptr;

    GameLogic::FrameworkParams PSAGame2DFramework::FrameworkParams = {};

    void PSAGame2DFramework::FrameworkRendererLowinit(const string& gl_version) {
        PsagLow::PsagSupGraphicsOper::PsagGraphicsSystemInit GLINIT;
        // in_logger funcptr => init.
        GLINIT.LoggerFunction(PushLogger);
        GLINIT.RendererInit(FrameworkGraphicsParams, gl_version);
    }

    bool PSAGame2DFramework::FrameworkRenderingGameScene() {
        // rendering background-shader sys.
        RendererBackFX->RenderingBackgroundModule();

        // run game_logic(game_scene).
        for (auto it = GAME_CORE_CLASS.begin(); it != GAME_CORE_CLASS.end(); ++it)
            it->second->LogicEventLoopGame(FrameworkParams);

        // update vertex_dataset state.
        SystemFrameUpdateNewState();
        return true;
    }

    bool PSAGame2DFramework::CoreFrameworkEvent() {
        bool CoreErrorFlag = PSAG_FALSE;
        ClacFrameTimeStepBegin();
        
        // update time_step.
        GraphicsEngineTimeStep = FrameworkParams.GameRunTimeSTEP;
        PhysicsEngineTimeStep  = FrameworkParams.GameRunTimeSTEP;
        ToolkitsEngineTimeStep = FrameworkParams.GameRunTimeSTEP;
        PhysicsSystemUpdateState();

        ActorModulesTimeStep = FrameworkParams.GameRunTimeSTEP;

        // calc & update global_matrix.
        float RoatioValue = (float)RenderingWindowSize.vector_x / (float)RenderingWindowSize.vector_y;
        FrameworkParams.CameraParams = &MatrixMainCamera;
        UpdateMatrixUniform(RoatioValue);

        // framework context event.
        RenderContextAbove();
        {
            RenderGuiContextA();
#if PSAG_DEBUG_MODE 
            if (RendererPostFX != nullptr) {
                // game scene => final shaders porcessing.
                auto GAME_SCENE = [&]() { return FrameworkRenderingGameScene(); };
                CoreErrorFlag |= !RendererPostFX->CaptureGameScene(GAME_SCENE);
                // render_pipline out_render.
                RendererPostFX->RenderingPostModule();
            }
#else
            auto GAME_SCENE = [&]() { return FrameworkRenderingGameScene(); };
            CoreErrorFlag |= !RendererPostFX->CaptureGameScene(GAME_SCENE);
            RendererPostFX->RenderingPostModule();
#endif
            RenderGuiContextB();
        }
        RenderContextBelow();
        
        // clac_frame global time_step.
        FrameworkParams.GameRunTimeSTEP = CalcFrameTimeStepEnd(RenderingBaseFPS);
        // glfw close_window flag & renderer block error.
        return !(CloseFlag() || CoreErrorFlag);
    }

    bool PSAGame2DFramework::CoreFrameworkInit(FrameworkSysVersion version) {
        bool CoreInitErrorFlag = PSAG_FALSE;
        // init_config_func => init...
        if (!FrameworkInitConfig(EngineGLSL)) {
            PushLogger(LogError, PSAGM_FRAME_CORE_LABEL, "core init: fatal error.");
            return false;
        }
        RenderingWindowSize = Vector2T<uint32_t>(WindowInitConfig.WindowSizeWidth, WindowInitConfig.WindowSizeHeight);

        // enable opengl profile config.
        FrameworkGraphicsParams.PROFILE_CONFIG = PSAG_FALSE;

        PushLogger(LogTrace, PSAGM_FRAME_CORE_LABEL, "core framework init,config...");
        // init => create => set_icon => vsync => sys_callback.
        CoreInitErrorFlag |= !GLFWwindowInit(GLFWversionArray, RendererMSAA, FrameworkGraphicsParams.PROFILE_CONFIG, RendererWindowFixed);
        CoreInitErrorFlag |= !GLFWwindowCreate(WindowInitConfig);

        GLFWwindowSetIcon(EngineIconImage);

        CoreInitErrorFlag |= !GLFWwindowVsync(PSAG_FALSE);
        CoreInitErrorFlag |= !GLFWwindowCallback();

        PsagLow::PsagSupGraphicsOper::GraphicsINFO GLINFO;
        GLINFO.LoggerPrintInformation = true;

        // init_window => init_renderer_low(opengl).
        FrameworkRendererLowinit(EngineGLSL);

        // print graph_device params.
        GLINFO.InfoPrintPlatformParameters();
        GLINFO.InfoGetGPUmemoryCapacity();
        GLINFO.InfoGetVertexUnitsMax();
        GLINFO.InfoGetTextureUnitsMax();
        GLINFO.InfoGetShaderUniform();

        // init create(alloc) OGL[LLRES], ver_system(s). 
        GraphicsEngineDataset::GLEngineStaticVertexData::LowLevelResourceCreate(PushLogger, 30);
        // init craete(alloc) OAL[LLRES].
        PsagLow::PsagSupAudioLLRES::LowLevelResourceCreate();

        // graphics system create.
        // dynamic vertex, static vertex, virtual textures.
        DynamicVertexDataObjectCreate();
        StaticVertexDataObjectCreate();
        VirtualTextureDataObjectCreate(RenderingVirTexBasicSize, VirTexturesMax);

        // create global matrix uniform_matrix.
        CreateMatrixUniform();

        // sound system create.
        CoreInitErrorFlag |= !CreateSoundDevice();
        // physics system create. default(debug?) world.
        CoreInitErrorFlag |= !PhysicsWorldCreate("DEFAULT_PHY_WORLD", Vector2T<float>());

        // create game2d post-shader & background(null)-shader.
        RendererPostFX = new GraphicsEngineFinal::PsagGLEngineFinal(RenderingWindowSize);
        RendererBackFX = new GraphicsEngineBackground::PsagGLEngineBackgroundNULL();

        // init,config imgui_context system.
        CoreInitErrorFlag |= !ImGuiContextInit(MainWindowObject, ImGuiInitConfig);

        // load pointer.
        FrameworkParams.ShaderParamsFinal = RendererPostFX->GetRenderParameters();
        FrameworkParams.WindowResolution = RenderingWindowSize;
        // non-create using default values.
        FrameworkParams.ShaderParamsBackground = RendererBackFX->GetRenderParameters();
       
        // registration dev_class, objects.
        InitializeRegistrationDev();
        // imit gui_logic(scene,gui).
        for (auto it = GAME_CORE_CLASS.begin(); it != GAME_CORE_CLASS.end(); ++it)
            it->second->LogicInitialization(RenderingWindowSize);

        *RenderingFrameColorPtr = DefaultFrameColor;

        PushLogger(LogTrace, PSAGM_FRAME_CORE_LABEL, "pomelo_star game2d version: %.8lf", version);
        // return init_items sum_flag.
        return !CoreInitErrorFlag;
    }

    bool PSAGame2DFramework::CoreFrameworkCloseFree() {
        bool CoreInitErrorFlag = PSAG_FALSE;
        // free framework oper...

        if (RendererBackFX != nullptr)  delete RendererBackFX;
        if (RendererPostFX != nullptr)  delete RendererPostFX;

        for (auto& ObjectItem : GAME_CORE_CLASS)
            delete ObjectItem.second;

        // graphics system init.
        DynamicVertexDataObjectDelete();
        StaticVertexDataObjectDelete();
        VirtualTextureDataObjectDelete();

        // sound system delete.
        CoreInitErrorFlag |= !DeleteSoundDevice();
        // physics system delete.
        CoreInitErrorFlag |= !PhysicsWorldDelete("DEFAULT_PHY_WORLD");

        ImGuiContextFree();
        CoreInitErrorFlag |= !PsagLow::PsagSupAudioLLRES::LowLevelResourceFree();
        CoreInitErrorFlag |= !GraphicsEngineDataset::GLEngineStaticVertexData::LowLevelResourceFree();
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