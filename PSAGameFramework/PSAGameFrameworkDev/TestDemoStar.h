// TestDemoStar. 2024_06_07 RCSZ.
// Framework Test-I: STAR

#ifndef __TESTDEMOSTAR_H
#define __TESTDEMOSTAR_H
#include "../PSAGameFrameworkCore/framework_dev_actor.h"
#include "../PSAGameFrameworkCore/framework_dev_manager.h"

extern const char* ActorFragPawn;
extern const char* ActorFragStar;

extern const char* ActorFragFX1;
extern const char* ActorFragFX2;
extern const char* ActorFragFX3;

class StarDemoFX {
protected:
    PsagActor::ActorsManager* ActorManager = nullptr;

    PsagActor::ActorRender* ActorShader1 = nullptr; // 能量球.
    PsagActor::ActorRender* ActorShader2 = nullptr; // 闪电.
    PsagActor::ActorRender* ActorShader3 = nullptr; // 能量束.

    GraphicsEngineParticle::PsagGLEngineParticle* ActorParticles = nullptr;

    float FxActorAnimTimer   = 0.0f;
    float FxActorTimer       = 0.0f;
    float FxActorCameraTimer = 0.0f;

    size_t FxActorUnique[3] = {};

    bool  FxActorFireFlag  = false;
    float FxActorFireAngle = 0.0f;
public:
    StarDemoFX(
        PsagActor::ActorsManager* Manager, Vector3T<PsagActor::ActorRender*> Shader,
        const Vector2T<float>& Position, float Angle
    );
    ~StarDemoFX();

    void StarDemoFxRender();
    void StarDemoFxFire();

    Vector2T<float> StarDemoFxCameraOffset(float* Scale, float* Rotate);
    float StarDemoFxProgress() {
        return FxActorAnimTimer / 20.0f;
    };
};

class StarDemoClass :public GameLogic::INTERFACE_DEVCLASS_GAME {
protected:
    Vector2T<float> CameraPosition = {};

    float GameTestMaxFPS = 0.0f;
    float CameraScale  = 1.0f;
    float CameraRotate = 0.0f;
    float FramerateParams[3] = {};

    int LightSamplerStep = 100;

    uint32_t EatStarCount      = NULL;
    size_t   EatStarCountTotal = NULL;

    GraphicsEngineParticle::PsagGLEngineParticle* AshesParticles = nullptr;
    
    // ######################## TEST ACTORS ########################

    PsagActor::ActorsManager TestGameActors;
    PsagActor::BricksManager TestGameBricks;

    ResUnique PawnActorCode = NULL;
    size_t NpcActorCode[3] = {};

    PsagActor::ActorRender* ActorShaderPawn = nullptr;
    PsagActor::ActorRender* ActorShaderStar = nullptr;

    PsagActor::ActorRender* BrickShader = nullptr;

    PsagActor::ActorRender* ActorShaderFX1 = nullptr; // 能量球着色器.
    PsagActor::ActorRender* ActorShaderFX2 = nullptr; // 闪电着色器.
    PsagActor::ActorRender* ActorShaderFX3 = nullptr; // 能量束着色器.

    StarDemoFX* ActorUltimateFX = nullptr;

    float UltimateSettingRotate = 0.0f;
    Vector2T<float> UltimateSettingPosition = {};

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