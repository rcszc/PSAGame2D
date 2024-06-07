// TestDemoStar. 2024_06_07 RCSZ.
// Framework Test-I: STAR

#ifndef __TESTDEMOSTAR_H
#define __TESTDEMOSTAR_H
#include "../PSAGameFrameworkCore/CoreFramework/psag_mainevent_support.h"

extern const char* ActorFragPawn;
extern const char* ActorFragNPC;

class TestClassStar :public GameLogic::INTERFACE_DEVCLASS_GAME, public PsagLow::PsagSupGraphicsLLRES {
protected:
    Vector2T<float> CameraPosition = {};

    float BloomRadius = 1.0f;
    float Visibility = 0.7f;
    float GameTestMaxFPS = 0.0f;
    float CameraScale = 1.0f;

    uint32_t EatNpcCount = NULL;

    GraphicsEngineParticle::PsagGLEngineParticle* AshesParticles = nullptr;
    
    // ######################## TEST ACTORS ########################

    GameCoreManager::GameActorActuatorManager TestGameActors;
    GameCoreManager::GameBrickActuatorManager TestGameBricks;

    ResUnique PawnActorCode = NULL;
    size_t NpcActorCode[3] = {};

    GameActorCore::GameActorShader* ActorShaderPawn = nullptr;
    GameActorCore::GameActorShader* ActorShaderNPC  = nullptr;
    GameActorCore::GameActorShader* BrickShader     = nullptr;

    void CreateBulletActor(Vector2T<float> PosBegin, Vector2T<float> PosSpeed);
    
public:
    // init: return flag: false:failed, true:success.
    bool LogicInitialization(const Vector2T<uint32_t>& WinSize);
    void LogicCloseFree();
    // loop: return flag: false:continue, true:exit.
    bool LogicEventLoopGame(GameLogic::FrameworkParams& RunningState);
    bool LogicEventLoopGui(GameLogic::FrameworkParams& RunningState);
};

#endif