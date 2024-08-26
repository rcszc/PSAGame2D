// v1demo_game. 2024_08_022 RCSZ.
// Framework DemoV1.

#ifndef __V1DEMO_GAME_H
#define __V1DEMO_GAME_H
#include "../../PSAGameFrameworkCore/framework_dev_actor.h"
#include "../../PSAGameFrameworkCore/framework_dev_manager.h"

extern const char* ShaderFragBackground;
extern const char* ShaderFragPawnActor;
extern const char* ShaderFragBulletActor;
extern const char* ShaderFragNPCActor;

extern const char* ActorFragFX1;
extern const char* ActorFragFX2;
extern const char* ActorFragFX3;

constexpr float PawnActorHPmax = 48800.0f;
constexpr float NpcActorHPmax = 2200.0f;

class PawnActorFX {
protected:
    PsagActor::ActorsManager* ActorManager = nullptr;

    PsagActor::ActorShader* ActorShader1 = nullptr; // 能量球.
    PsagActor::ActorShader* ActorShader2 = nullptr; // 闪电.

    GraphicsEngineParticle::PsagGLEngineParticle* ActorParticles = nullptr;

    float FxActorAnimTimer = 0.0f;
    float FxActorTimer     = 0.0f;

    size_t FxActorUnique[2] = {};
public:
    PawnActorFX(
        PsagActor::ActorsManager* Manager, Vector2T<PsagActor::ActorShader*> Shader, 
        const Vector2T<float>& Position
    );
    ~PawnActorFX();

    void PawnActorFxRender();
};

class PsaGameV1Demo :public GameLogic::INTERFACE_DEVCLASS_GAME {
protected:
    PsagActor::ShaderManager* DemoShaders      = nullptr;
    PsagActor::BricksManager* DemoArchitecture = nullptr;
    PsagActor::ActorsManager* DemoActors       = nullptr;

    PsagActor::PresetScript SYS_PRESET_SC = {};

    // PActor 生命恢复间隔计时器, 开火计时器.
    PsagManager::Tools::Timer::GameCycleTimer* PActorHP   = nullptr;
    PsagManager::Tools::Timer::GameCycleTimer* PActorFIRE = nullptr;

    PsagManager::Fx::Particle* DemoParticlePActor1 = nullptr; // 回血粒子.
    PsagManager::Fx::Particle* DemoParticlePActor2 = nullptr; // 换弹粒子.
    PsagManager::Fx::Particle* DemoParticleBullet  = nullptr; // 子弹击中粒子.

    PsagManager::Tools::Pawn::GamePlayerPawn*        PlayerPawn   = nullptr;
    PsagManager::Tools::Camera::GamePlayerComaeraMP* PlayerCamera = nullptr;

    PsagManager::FxView::TextureViewImage* GuiViewImage = nullptr;

    float CameraShakeValue = 0.0f;
    float PawnActorBullet = 70.0f;

    ResUnique PawnActorUnqiue = NULL;

    void GameInitActorsShader(const Vector2T<uint32_t>& w_size);
    void GameInitPawnActor();
    void GameInitBoundary();
    void GameInitParticleSystem(const Vector2T<uint32_t>& w_size);

    void GameCreateBulletPawn(const Vector2T<float>& position, float angle);
    void GameCreateBulletNPC(const Vector2T<float>& position, float angle);

    void GameCreateNPC(const Vector2T<float>& position);

    void GameCreateParticlesPActorADHP(float number);
    void GameCreateParticlesPActorCAMM(float number);

    // bullet mode: 0: pawn_actor, 1: npc_actors.
    void GameCreateParticlesBullet(float number, const Vector2T<float>& pos, int mode = 0);

public:
    // init: return flag: false:failed, true:success.
    bool LogicInitialization(const Vector2T<uint32_t>& WinSize);
    void LogicCloseFree();
    // loop: return flag: false:continue, true:exit.
    bool LogicEventLoopGame(GameLogic::FrameworkParams& RunningState);
};

#endif