// ModuleDebug.

#ifndef __MODULEDEBUG_H
#define __MODULEDEBUG_H
#include "../PSAGameFrameworkCore/CoreFramework/psag_mainevent_support.h"

class ModuleDebugClass :public GameLogic::INTERFACE_DEVCLASS_GAME {
protected:
    float FramerateParams[3] = {};

public:
    // init: return flag: false:failed, true:success.
    bool LogicInitialization(const Vector2T<uint32_t>& WinSize);
    void LogicCloseFree();
    // loop: return flag: false:continue, true:exit.
    bool LogicEventLoopGame(GameLogic::FrameworkParams& RunningState);
    bool LogicEventLoopGui(GameLogic::FrameworkParams& RunningState);
};

#endif