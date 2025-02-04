// psag_game_lab. RCSZ.
// framework game test.

#ifndef __PSAG_GAME_LAB_H
#define __PSAG_GAME_LAB_H
#include "../../PSAGameFrameworkCore/framework_header_actor.h"
#include "../../PSAGameFrameworkCore/framework_header_manager.h"

namespace Mptr = PsagManager::PtrMAG;

class PSAGameFrameworkLab :public GameLogic::INTERFACE_DEVCLASS_GAME {
protected:
    Mptr::DefinePointer<PsagManager::FxView::TextureViewImage> Texture[4] = {};
    uint32_t TextureIndex[4] = {};
public:
    // init: return flag: false:failed, true:success.
    bool LogicInitialization(const Vector2T<uint32_t>& WinSize);
    void LogicCloseFree();
    // loop: return flag: false:continue, true:exit.
    bool LogicEventLoopGame(GameLogic::FrameworkParams& RunningState);
};

#endif