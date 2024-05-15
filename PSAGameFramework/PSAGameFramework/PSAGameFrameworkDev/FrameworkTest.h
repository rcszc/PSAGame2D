// FrameworkTest.

#ifndef __FRAMEWORKTEST_H
#define __FRAMEWORKTEST_H
#include "../PSAGameFrameworkCore/CoreFramework/psag_mainevent_support.h"

class DevTestClass :public GameLogic::INTERFACE_DEVCLASS_GAME, public PsagLow::PsagSupGraphicsLLRES {
protected:

    GraphicsEngineParticle::PsagGLEngineParticle* AshesParticles = nullptr;
    size_t CountTimer = NULL;

    Vector4T<float> TestColor = {};

    ImVec4 BgBlendColorSet = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 ColorAnimInter  = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);

    float BloomRadius = 1.0f;
    float BgVisibilitySet = 0.7f;
    float BgBlendStrengthSet = 0.2f;

    GraphicsEnginePVFX::PsagGLEngineFxCaptureView* TestCaptureView = nullptr;
    GraphicsEnginePVFX::PsagGLEngineFxSequence*    TestSequence    = nullptr;

public:
    // init: return flag: false:failed, true:success.
    bool LogicInitialization();
    void LogicCloseFree();
    // loop: return flag: false:continue, true:exit.
    bool LogicEventLoopGame(GameLogic::FrameworkParams& RunningState);
    bool LogicEventLoopGui(GameLogic::FrameworkParams& RunningState);
};

#endif