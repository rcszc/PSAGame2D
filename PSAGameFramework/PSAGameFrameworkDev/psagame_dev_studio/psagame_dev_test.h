// v1demo_game. 2024_10_12 RCSZ.

#ifndef __PSAGAME_DEV_TEST_H
#define __PSAGAME_DEV_TEST_H
#include "../../PSAGameFrameworkCore/framework_header_actor.h"
#include "../../PSAGameFrameworkCore/framework_header_manager.h"

template<typename T>
struct ThreadSafeResource {
    T Resource = {};
    std::mutex ResourceMutex = {};
};

struct RunSettingLimitParams {
    size_t RunMaxCycles = NULL; // tick
    size_t RunMaxMemory = NULL; // bytes
};

class GLOBAL_SAFE_RESOURCE {
protected:
    static ThreadSafeResource<RunSettingLimitParams> RunThreadParams;
    static ThreadSafeResource<std::string>           RunThreadStrLog;

    static std::atomic<bool> ThreadFlagStart;
    static std::atomic<bool> ThreadFlagExit;

    static std::atomic<size_t> RunThreadTickCount;
    static std::atomic<size_t> RunThreadDataCount;
};

namespace PSA_PANEL {
    using SYS_CLOCK = std::chrono::system_clock::time_point;

    class MainControlPanel :public GLOBAL_SAFE_RESOURCE {
    protected:
        SYS_CLOCK StartTimePoint = std::chrono::system_clock::now();
        SYS_CLOCK RunTimePoint   = std::chrono::system_clock::now();

        int32_t SettingRunCycles    = NULL;
        float   SettingRunMemoryMib = 0.0f;
        
        void RunThreadFlagsProcess();
    public:
        void RenderPanel();
    };
}

class PsaGameDevTest :public GameLogic::INTERFACE_DEVCLASS_GAME {
protected:
    PSA_PANEL::MainControlPanel* MainControlPanel = nullptr;

    PsagManager::GuiTools::Large::ImMegaPlotsDataView* TestImGuiPlot = nullptr;

    PsagActor::BricksManager* TestArchitecture = nullptr;
    PsagActor::ActorShader* TestShader = nullptr;
   
public:
    // init: return flag: false:failed, true:success.
    bool LogicInitialization(const Vector2T<uint32_t>& WinSize);
    void LogicCloseFree();
    // loop: return flag: false:continue, true:exit.
    bool LogicEventLoopGame(GameLogic::FrameworkParams& RunningState);
};

#endif