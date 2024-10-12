// v1demo_game. 2024_10_12 RCSZ.

#ifndef __PSAGAME_DEV_TEST_H
#define __PSAGAME_DEV_TEST_H
#include "../../PSAGameFrameworkCore/framework_header_actor.h"
#include "../../PSAGameFrameworkCore/framework_header_manager.h"

class PsaGameDevTest :public GameLogic::INTERFACE_DEVCLASS_GAME {
protected:
   
public:
    // init: return flag: false:failed, true:success.
    bool LogicInitialization(const Vector2T<uint32_t>& WinSize);
    void LogicCloseFree();
    // loop: return flag: false:continue, true:exit.
    bool LogicEventLoopGame(GameLogic::FrameworkParams& RunningState);
};

#endif