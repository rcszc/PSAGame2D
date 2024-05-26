// FrameworkTest.

#ifndef __FRAMEWORKTEST_H
#define __FRAMEWORKTEST_H
#include "../PSAGameFrameworkCore/CoreFramework/psag_mainevent_support.h"

extern const char* ActorFragPawn;
extern const char* ActorFragBullet;
extern const char* ActorFragNPC;
extern const char* ActorFragWall;

class DevTestClass :public GameLogic::INTERFACE_DEVCLASS_GAME, public PsagLow::PsagSupGraphicsLLRES {
protected:

    GraphicsEngineParticle::PsagGLEngineParticle* AshesParticles = nullptr;
    size_t CountTimer = NULL;

    Vector4T<float> TestColor = Vector4T<float>(1.0f, 1.0f, 1.0f, 1.0f);

    ImVec4 BgBlendColorSet = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);
    ImVec4 ColorAnimInter  = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);

    float BloomRadius = 1.0f;
    float BgVisibilitySet = 0.7f;
    float BgBlendStrengthSet = 0.2f;

    GraphicsEnginePVFX::PsagGLEngineFxCaptureView* TestCaptureView = nullptr;
    GraphicsEnginePVFX::PsagGLEngineFxSequence*    TestSequence    = nullptr;

    // ######################## TEST ACTORS ########################

    GameActorManager::GameActorActuatorManager TestGameActors;

    size_t PawnActorCode = NULL;
    size_t NpcActorCode[3] = {};

    GameActorCore::GameActorShader* ActorShaderPawn = nullptr;
    GameActorCore::GameActorShader* ActorShaderBullet = nullptr;
    GameActorCore::GameActorShader* ActorShaderNPC = nullptr;
    GameActorCore::GameActorShader* ActorShaderWall = nullptr;

    void CreateNpcActor(float max_hp);
    void CreateBulletActor(Vector2T<float> PosBegin, Vector2T<float> PosSpeed);
    void HealthHandlerFuncNPC(const GameActorCore::HealthFuncParams& params);

public:
    // init: return flag: false:failed, true:success.
    bool LogicInitialization(const Vector2T<uint32_t>& WinSize);
    void LogicCloseFree();
    // loop: return flag: false:continue, true:exit.
    bool LogicEventLoopGame(GameLogic::FrameworkParams& RunningState);
    bool LogicEventLoopGui(GameLogic::FrameworkParams& RunningState);
};

#endif