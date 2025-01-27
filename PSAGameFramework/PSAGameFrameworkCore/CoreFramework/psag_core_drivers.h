// psag_core_drivers. RCSZ.
// framework core drivers, mid_layer.
// program 'main' entry point.

#ifndef __PSAG_CORE_DRIVERS_H
#define __PSAG_CORE_DRIVERS_H

#define ENABLE_LOWMODULE_GRAPHICS
#define ENABLE_LOWMODULE_FILESYS
#define ENABLE_LOWMODULE_AUDIO
#define ENABLE_LOWMODULE_THREAD
#define ENABLE_LOWMODULE_WINDOWGUI
#include "psag_lowlevel_support.h"
// framework core actor & manager.
#include "../CoreGameActor/psag_actor_module.hpp"
#include "../CoreGameManager/psag_manager_module.hpp"

namespace GameLogic {
    // framework run parameters.
    struct FrameworkParams {
        GraphicsEngineMatrix::MatrixTransParams* CameraParams;

        GraphicsEngineFinal::FinalFxParameters*     ShaderParamsFinal;
        GraphicsEngineBackground::BackFxParameters* ShaderParamsBackground;

        Vector2T<uint32_t> WindowResolution;
        float GameRunTimeSTEP;
    };
    
    // frameowrk game_logic interface.
    class INTERFACE_DEVCLASS_GAME {
    public:
        // init: return flag: false:failed, true:success.
        virtual bool LogicInitialization(const Vector2T<uint32_t>& WinSize) = 0;
        virtual void LogicCloseFree() = 0;
        // loop: return flag: false:continue, true:exit.
        virtual bool LogicEventLoopGame(FrameworkParams& RunningState) = 0;
        // 废弃: 独立的Gui事件渲染. RCSZ [20240826]
        // virtual bool LogicEventLoopGui(FrameworkParams& RunningState) = 0;
    };
}

namespace PsagMainEvent {
    StaticStrLABEL PSAGM_MAIN_EVENT_LABEL = "PSAG_M_MAINEVENT";

    struct Event {
        std::string          EventName;
        size_t               EventUniqueID;
        std::function<int()> EventTask;
    };

    class MainAsyncTask {
    protected:
        // generate event.task unique_id.
        PSAG_SYS_GENERATE_KEY SystemGenUID;
        
        std::vector<Event>            EventsArray   = {};
        std::queue<int>               EventsResults = {};
        std::vector<std::future<int>> FutureEvent   = {};

        std::atomic<bool> RunningFlag = false;
        uint32_t CYCLES_SLEEP_TIME = NULL;

        void AsyncEventStart();
    public:
        MainAsyncTask(uint32_t cyctime) : CYCLES_SLEEP_TIME(cyctime) {}

        void TasksStart();
        void AsyncTaskADD(std::string name, std::function<int()> event);

        std::queue<int>* GetTasksResults() {
            // results(queue).
            return &EventsResults;
        }
    };
}

namespace PsagFrameworkStart {
    class GameLogic::INTERFACE_DEVCLASS_GAME;
    StaticStrLABEL PSAGM_DRIVE_START_LABEL = "PSAG_M_START";

#define PSAGM_FLAG_FRAMEWORK 0xA0
#define PSAGM_FLAG_NETWORK   0xB0
#define PSAGM_FLAG_EXTMODULE 0xC0

    // core middle framework interface.
    class CoreMidFrameworkBase {
    public:
        virtual bool CoreFrameworkInit(FrameworkSysVersion version) = 0;
        virtual bool CoreFrameworkEvent()     = 0;
        virtual bool CoreFrameworkCloseFree() = 0;

        virtual void GameRegistrationObject(const char* class_name, const char* unique_name, GameLogic::INTERFACE_DEVCLASS_GAME* rclass) = 0;
        virtual void GameDeleteObject(const char* unique_name) = 0;
        virtual GameLogic::INTERFACE_DEVCLASS_GAME* GameFindRegObject(const char* unique_name) = 0;
    };
    namespace System {
        extern CoreMidFrameworkBase* GLOBAL_FMC_OBJECT;
    }

    // core middle module.network interface.
    class CoreMidFrameworkNetBase {
    public:
        virtual bool CoreFrameworkInit()      = 0;
        virtual bool CoreFrameworkEvent()     = 0;
        virtual void CoreFrameworkDataEvent() = 0;
        virtual bool CoreFrameworkCloseFree() = 0;
    };

    // core middle module.external interface.
    class CoreMidExtModuleBase {
    public:
        virtual bool CoreFrameworkInit(FrameworkSysVersion version) = 0;
        virtual bool CoreFrameworkEvent()     = 0;
        virtual void CoreFrameworkDataEvent() = 0;
        virtual bool CoreFrameworkCloseFree() = 0;
    };

    class CorePsagMainStart {
    protected:
        PsagMainEvent::MainAsyncTask* StartEventsLoop = nullptr;

        CoreMidFrameworkBase*                 CoreFramewokObj  = {};
        std::vector<CoreMidFrameworkNetBase*> CoreNetworkObj   = {};
        std::vector<CoreMidExtModuleBase*>    CoreExtModuleObj = {};

        void StartCoreObjectsRes(FrameworkSysVersion version);
    public:
        void StartPsaGame(FrameworkSysVersion version, uint32_t cyctiem);
        int  FreeFramework();

        void UniqueFrameworkObj(CoreMidFrameworkBase* objptr);
        bool AddItemNetworkObj(CoreMidFrameworkNetBase* objptr);
        bool AddItemExtModuleObj(CoreMidExtModuleBase* objptr);
    };
}

// framework program start anim. 
namespace PsagFrameworkAnim {
    StaticStrLABEL PSAGM_DRIVE_ANIM_LABEL = "PSAG_M_START_ANIM";

    struct HeaderImage {
        PsagTexture     ImageTexture = {};
        Vector4T<float> ImageBlend   = {};
    };
    class PsagStartAnimResource {
    protected:
        static std::vector<HeaderImage> HeaderImages;
        static float ImagesPlayTimeSec;
        static float ImagesLerpTrans;
    };

    class PsagStartAnimPlayer :public PsagStartAnimResource {
    protected:
        Vector2T<float> WindowSize = Vector2T<float>(1.0f, 1.0f);
        // 0: blend.target.end, 1: blend.target.begin
        Vector4T<float> LerpTargetBlend[2] = {
            Vector4T<float>(0.0f, 0.0f, 0.0f, 0.0f),
            Vector4T<float>(1.0f, 1.0f, 1.0f, 1.0f)
        };
        size_t ImagesIndexCount = 0;
        size_t TragetIndexCount = 1;

        std::chrono::system_clock::time_point TimerTemp = {};
        uint32_t LerpStatusFlag = 0;
    public:
        PsagStartAnimPlayer(const Vector2T<float>& win_size) : WindowSize(win_size) {
            PSAG_LOGGER::PushLogger(LogInfo, PSAGM_DRIVE_ANIM_LABEL, "create start_anim player.");
        };
        ~PsagStartAnimPlayer();

        bool PlayStartAnimation();
    };

    class PsagStartAnimLoad :public PsagStartAnimResource {
    public:
        bool AnimImageADD(const ImageRawData& image);

        void SettingPlayTime(float time_s) { ImagesPlayTimeSec = time_s > 0.0f ? time_s : 1.0f; }
        void SettingLerpSpeed(float speed) { ImagesLerpTrans = speed > 0.0f ? speed : 1.0f; }
    };
}

namespace PsagFrameworkCore {
    StaticStrLABEL PSAGM_DRIVE_CORE_LABEL = "PSAG_M_CORE";
    // global framework components.
    namespace PsagManage {
        class SysBackground;
    }

    // @pomelo_star game2d framework core drivers class. RCSZ.
    class PSAGame2DFramework :public PsagFrameworkStart::CoreMidFrameworkBase,
        // init & free sound_system.
        public PsagLow::PsagSupAudioSystem::PsagAudioSystemDevice, // sound device init.
        public PsagLow::PsagSupAudioLLRES,                         // sound resource manager.
        public PsagLow::PsagSupWindowGuiEventOGLFW,
        public PsagLow::PsagSupWindowGuiEventIMGUI,
        // update global matrix.
        public GraphicsEngineMatrix::PsagGLEngineMatrix,
        // public PsagLow::PsagSupGraphicsLLRES. => "GLEngineVertexDyData".
        // init vertex_group_mag(vao,vbo) & vir_texture system.
        public GraphicsEngineDataset::GLEngineDynamicVertexData, // vertex_group(dyamic_flag).
        public GraphicsEngineDataset::GLEngineStaticVertexData,  // vertex_group(static_flag).
        public GraphicsEngineDataset::GLEngineVirTextureData,    // virtual textures.
        // init physics system.
        public PhysicsEngine::PhyEngineCoreDataset,
        // system time step.
        public __GRAPHICS_ENGINE_TIMESETP,
        public __ACTOR_MODULES_TIMESTEP,
        // public __PHYSICS_ENGINE_TIMESETP. => "PhysicsEngine::PhyEngineCoreDataset".
        public __TOOLKITS_ENGINE_TIMESETP
    {
    private:
        static GraphicsEngineFinal::PsagGLEngineFinal* RendererPostFX;
        static GraphicsEngineBackground::PsagGLEngineBackgroundBase* RendererBackFX;

        static GameLogic::FrameworkParams FrameworkParams;
        // friend class:
        friend class PsagManage::SysBackground;
    protected:
        // graphics init parameters.
        INIT_PARAMETERS FrameworkGraphicsParams = {};
        // user registration game_logic class data.
        std::unordered_map<std::string, GameLogic::INTERFACE_DEVCLASS_GAME*> GAME_CORE_CLASS = {};

        PsagLow::PsagSupWindowGuiConfigWin WindowInitConfig = {};
        PsagLow::PsagSupWindowGuiConfigGui ImGuiInitConfig  = {};

        // virtual textures_level number max.
        GraphicsEngineDataset::VirTexturesGenParams VirTexturesMax = {};

        // renderer init params.
        uint32_t RendererMSAA        = 1;
        bool     RendererWindowFixed = false;

        // renderer event_loop params.
        Vector4T<float>* RenderingFrameColorPtr = &RenderClearColor;

        Vector2T<uint32_t> RenderingWindowSize      = {};
        Vector2T<uint32_t> RenderingVirTexBasicSize = {};

        PsagFrameworkAnim::PsagStartAnimPlayer* StartAnim = nullptr;

        // time step (fps)benchmark.
        float RenderingBaseFPS = 120.0f;
        // render content func.
        std::function<bool()> RENDER_CONTENT = []() { return false; };

        bool FrameworkInitConfig(const std::string& gl_version);
        void FrameworkRendererLowinit(const std::string& gl_version);
        // run,update,rendering game.
        bool FrameworkRenderingGameScene();
        bool FrameworkRendering();
    public:

        bool CoreFrameworkInit(FrameworkSysVersion version) override;
        bool CoreFrameworkEvent() override;
        bool CoreFrameworkCloseFree() override;

        void GameRegistrationObject(const char* class_name, const char* unique_name, GameLogic::INTERFACE_DEVCLASS_GAME* rclass) override;
        void GameDeleteObject(const char* unique_name) override;
        GameLogic::INTERFACE_DEVCLASS_GAME* GameFindRegObject(const char* unique_name) override;

        // file:"framework_register.cpp"[20240501].
        void InitializeRegistrationDev();
    };

    namespace PsagManage {
        StaticStrLABEL PSAGM_FRAME_COREMAG_LABEL = "PSAG_M_COREMAG";

        // create game background_system.
        class SysBackground :protected PSAGame2DFramework {
        protected:
            std::vector<ImageRawData> LayersRawImages = {};
        public:
            bool LayerDataPush(const ImageRawData& layer_image);
            void CreateBackground(const Vector2T<uint32_t>& resolution);
        };
    }
}
// game system namespace
namespace PsagGameSys = PsagFrameworkCore::PsagManage;

// register game developer_class.
namespace RegisterDevClass {
    StaticStrLABEL PSAGM_DRIVE_REG_LABEL = "PSAG_M_REGISTER";

    class ReflectionFactory {
    public:
        using CreateFunction = std::function<GameLogic::INTERFACE_DEVCLASS_GAME* ()>;
        // factory process.
        static std::unordered_map<std::string, CreateFunction>& GetRegistry() {
            static std::unordered_map<std::string, CreateFunction> Registry;
            return Registry;
        }
    };

    template<typename T>
    class Registrar {
    public:
        Registrar(const std::string& class_name) {
            auto CreateFunction = []()->GameLogic::INTERFACE_DEVCLASS_GAME* {
                return new T();
            };
            // register.
            auto& Registry = ReflectionFactory::GetRegistry();
            Registry[class_name] = CreateFunction;
        }
    };

    class PsagReflectionSYS {
    public:
        template <typename Object>
        // framework devobj_object 获取对象.
        Object* ReflectionObjectFind(const char* UniqueName) {
            // get registration object.
            GameLogic::INTERFACE_DEVCLASS_GAME* ObjectPtr = 
                PsagFrameworkStart::System::GLOBAL_FMC_OBJECT->GameFindRegObject(UniqueName);
            // check object ptr & object types.
            if (ObjectPtr == nullptr) {
                PSAG_LOGGER::PushLogger(LogError, PSAGM_DRIVE_REG_LABEL, "failed: object = nullptr.");
                return nullptr;
            }
            if (typeid(*ObjectPtr) != typeid(Object)) {
                PSAG_LOGGER::PushLogger(LogError, PSAGM_DRIVE_REG_LABEL, "failed: object is not of the expected type.");
                return nullptr;
            }
            return dynamic_cast<Object*>(ObjectPtr);
        }

        // framework gamedev_object 创建对象.
        void ReflectionObjectCreate(const char* ClassName, const char* UniqueName);
        // framework gamedev_object 销毁对象.
        void ReflectionObjectDelete(const char* UniqueName);
    };
    // framework reflection call global objects.
    extern PsagReflectionSYS GAMEREF;
}
// 注册用户类 (inheritance "INTERFACE_DEVCLASS_GAME").
#define PSAGCLASS(UserClass) \
    inline static RegisterDevClass::Registrar<UserClass> UserClass##Registrar(#UserClass)

#endif