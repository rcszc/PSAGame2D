// TestDemoStar. 2024_06_07 RCSZ.
// Framework Test-I: STAR

#ifndef __TESTDEMOSTAR_H
#define __TESTDEMOSTAR_H
#include "../PSAGameFrameworkCore/CoreFramework/psag_mainevent_support.h"

extern const char* ActorFragPawn;
extern const char* ActorFragStar;
extern const char* ActorFragNPC;

class StarDemoClass :public GameLogic::INTERFACE_DEVCLASS_GAME {
protected:
    Vector2T<float> CameraPosition = {};

    float GameTestMaxFPS = 0.0f;
    float CameraScale = 1.0f;
    float FramerateParams[3] = {};

    int LightSamplerStep = 100;

    uint32_t EatStarCount      = NULL;
    size_t   EatStarCountTotal = NULL;

    GraphicsEngineParticle::PsagGLEngineParticle* AshesParticles = nullptr;
    
    // ######################## TEST ACTORS ########################

    GameCoreManager::GameActorActuatorManager TestGameActors;
    GameCoreManager::GameBrickActuatorManager TestGameBricks;

    ResUnique PawnActorCode = NULL;
    size_t NpcActorCode[3] = {};

    GameActorCore::GameActorShader* ActorShaderPawn = nullptr;
    GameActorCore::GameActorShader* ActorShaderStar = nullptr;
    GameActorCore::GameActorShader* BrickShader     = nullptr;

    void CreateStarActor(Vector2T<float> PosBegin, Vector2T<float> PosSpeed);
    void CreateRandomStarActors(size_t number);
    
public:
    // init: return flag: false:failed, true:success.
    bool LogicInitialization(const Vector2T<uint32_t>& WinSize);
    void LogicCloseFree();
    // loop: return flag: false:continue, true:exit.
    bool LogicEventLoopGame(GameLogic::FrameworkParams& RunningState);
    bool LogicEventLoopGui(GameLogic::FrameworkParams& RunningState);
};

#endif