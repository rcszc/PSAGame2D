// zero_project_gui. RCSZ.
// game module(desc): game main ui.

#ifndef __ZERO_PROJECT_GUI_H
#define __ZERO_PROJECT_GUI_H
#include "base_define_header.hpp"

class ZPGameGuiMain :public GameLogic::INTERFACE_DEVCLASS_GAME,
    public ZPGameGlobale {
protected:
    std::unordered_map<size_t, PsagManager::FxView::TextureViewImage*> 
        ControlSlotDrawTextures = {};
    float LerpDamageView = 0.0f;
public:
    // init: return flag: false:failed, true:success.
    bool LogicInitialization(const Vector2T<uint32_t>& WinSize);
    void LogicCloseFree();
    // loop: return flag: false:continue, true:exit.
    bool LogicEventLoopGame(GameLogic::FrameworkParams& RunningState);
};

#endif