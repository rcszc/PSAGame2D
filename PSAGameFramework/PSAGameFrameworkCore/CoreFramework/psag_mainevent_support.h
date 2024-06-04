// psag_mainevent_support. RCSZ.
// framework main support(core), mid_level.

#ifndef __PSAG_MAINEVENT_SUPPORT_H
#define __PSAG_MAINEVENT_SUPPORT_H

#define ENABLE_LOWMODULE_GRAPHICS
#define ENABLE_LOWMODULE_FILESYS
#define ENABLE_LOWMODULE_SOUND
#define ENABLE_LOWMODULE_THREAD
#define ENABLE_LOWMODULE_WINDOWGUI
#include "psag_lowlevel_support.h"
// framework core actor.
#include "../CoreGameActor/psag_actor_module.hpp"

namespace GameLogic {
    // framework run parameters.
    struct FrameworkParams {
        GraphicsEnginePost::PostFxParameters*       PostShaderParams;
        GraphicsEngineBackground::BackFxParameters* BackShaderParams;

        Vector2T<uint32_t> WindowResolution;
        float GameRunTimeStep;
    };

    // frameowrk game_logic interface.
    class INTERFACE_DEVCLASS_GAME {
    public:
        // init: return flag: false:failed, true:success.
        virtual bool LogicInitialization(const Vector2T<uint32_t>& WinSize) = 0;
        virtual void LogicCloseFree() = 0;
        // loop: return flag: false:continue, true:exit.
        virtual bool LogicEventLoopGame(FrameworkParams& RunningState) = 0;
        virtual bool LogicEventLoopGui(FrameworkParams& RunningState)  = 0;
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
        // generate event_task unique_id.
        PSAG_SYSGEN_TIME_KEY SystemGenUID;
        
        std::vector<Event>            EventsArray   = {};
        std::queue<int>               EventsResults = {};
        std::vector<std::future<int>> FutureEvent   = {};

        uint32_t CyclesSleepTime = NULL;
        bool RunningFlag = false;

        void AsyncEventStart();
    public:
        MainAsyncTask(uint32_t cyctime) : CyclesSleepTime(cyctime) {}

        void TasksStart();
        void AsyncTaskAdd(std::string name, std::function<int()> event);

        std::queue<int>* GetTasksResults() {
            // results(queue).
            return &EventsResults;
        }
    };
}

namespace PsagFrameworkStart {
    class GameLogic::INTERFACE_DEVCLASS_GAME;
    StaticStrLABEL PSAGM_FRAME_START_LABEL = "PSAG_M_START";

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

    // core middle framework network interface.
    class CoreMidFrameworkNetBase {
    public:
        virtual bool CoreFrameworkInit()      = 0;
        virtual bool CoreFrameworkEvent()     = 0;
        virtual void CoreFrameworkDataEvent() = 0;
        virtual bool CoreFrameworkCloseFree() = 0;
    };

    // core middle external_module interface.
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

namespace PsagFrameworkCore {
    StaticStrLABEL PSAGM_FRAME_CORE_LABEL = "PSAG_M_CORE";
    namespace PsagManage {
        class SysBackground;
        class SysFrontMask;
    }

    class PSAGame2DFramework :public PsagFrameworkStart::CoreMidFrameworkBase,
        // init & free sound_system.
        public PsagLow::PsagSupSoundSystem::__PsagSoundDeviceHandle,
        public PsagLow::PsagSupWindowGuiEventOglfw,
        public PsagLow::PsagSupWindowGuiEventImGui,
        // public PsagLow::PsagSupGraphicsLLRES, (与"GLEngineVertexDyData"冲突)
        // init vertex_group_mag(vao,vbo) & vir_texture system.
        public GraphicsEngineDataset::GLEngineDyVertexData,   // dyamic vertex.
        public GraphicsEngineDataset::GLEngineStcVertexData,  // static vertex.
        public GraphicsEngineDataset::GLEngineSmpTextureData, // textures.
        // init physics system.
        public PhysicsEngine::PhyEngineCoreDataset,
        // system time step.
        public __GRAPHICS_ENGINE_TIMESETP,
        public __ACTOR_MODULES_TIMESTEP,
        // system camera position get.
        public __ACTOR_MODULES_CAMERAPOS
        // public __PHYSICS_ENGINE_TIMESETP, (与"PhysicsEngine::PhyEngineCoreDataset"冲突)
    {
    private:
        static GraphicsEnginePost::PsagGLEnginePost* RendererPostFX;
        static GraphicsEngineBackground::PsagGLEngineBackground* RendererBackFX;

        GraphicsEngineBackground::BackFxParameters BackgroundParams = {};

        // friend class:
        friend class PsagManage::SysBackground;
    protected:
        INIT_PARAMETERS FrameworkGraphicsParams = {}; // graphics init parameters.
        GameLogic::FrameworkParams FrameworkParams = {};
        // user registration game_logic class data.
        std::unordered_map<std::string, GameLogic::INTERFACE_DEVCLASS_GAME*> GAME_CORE_CLASS = {};

        PsagLow::PsagSupWindowGuiConfigWin WindowInitConfig = {};
        PsagLow::PsagSupWindowGuiConfigGui ImGuiInitConfig = {};

        // renderer init params.
        uint32_t RendererMSAA        = 1;
        bool     RendererWindowFixed = false;

        // renderer event_loop params.
        Vector4T<float>* RenderingFrameColorPtr = &RenderBgColor;
        Vector2T<uint32_t> RenderingWinSize = {};
        Vector2T<uint32_t> RenderingVirTextureSizeBase = {};
        // time step (fps)benchmark.
        float RenderingBaseFPS = 120.0f;

        void FrameworkInitConfig(const std::string& gl_version);
        void FrameworkRendererLowinit(const std::string& gl_version);

        bool FrameworkRenderingGameScene();
        bool FrameworkRenderingGameGui();
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
        // non-eventloop.
        class SysBackground :protected PSAGame2DFramework {
        protected:
            std::vector<ImageRawData> LayersRawImages = {};
        public:
            bool LayerDataPush(const ImageRawData& layer_image);
            void CreateBackground(const Vector2T<uint32_t>& resolution);
        };
    }
}
// game_sys namespace
namespace PsagGameSys = PsagFrameworkCore::PsagManage;

// register game developer_class.
namespace RegisterDevClass {
    StaticStrLABEL PSAGM_FRAME_DEVREG_LABEL = "PSAG_M_REGISTER";

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

    class ProfxReflection {
    public:
        template <typename Object>
        // framework devobj_object 获取对象.
        Object* ReflectionObjectFind(const char* UniqueName) {
            // get registration object.
            GameLogic::INTERFACE_DEVCLASS_GAME* ObjectPtr = PsagFrameworkStart::System::GLOBAL_FMC_OBJECT->GameFindRegObject(UniqueName);
            if (ObjectPtr == nullptr) {
                PSAG_LOGGER::PushLogger(LogError, PSAGM_FRAME_DEVREG_LABEL, "failed: object = nullptr.");
                return nullptr;
            }
            if (typeid(*ObjectPtr) != typeid(Object)) {
                PSAG_LOGGER::PushLogger(LogError, PSAGM_FRAME_DEVREG_LABEL, "failed: object is not of the expected type.");
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
    extern ProfxReflection GAMEREF;
}
// 注册用户类 (inheritance "INTERFACE_DEVCLASS_GAME").
#define PSAGCLASS(UserClass) \
    inline static RegisterDevClass::Registrar<UserClass> UserClass##Registrar(#UserClass)

#endif