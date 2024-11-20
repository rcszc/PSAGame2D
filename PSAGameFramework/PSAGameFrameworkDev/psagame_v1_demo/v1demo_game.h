// v1demo_game. 2024_08_22 RCSZ.
// Framework DemoV1.

#ifndef __V1DEMO_GAME_H
#define __V1DEMO_GAME_H
#include "../../PSAGameFrameworkCore/framework_header_actor.h"
#include "../../PSAGameFrameworkCore/framework_header_manager.h"

extern const char* ShaderFragBackground;
extern const char* ShaderFragPawnActor;
extern const char* ShaderFragBulletActor;
extern const char* ShaderFragNPCActor;

constexpr float PawnActorHPmax = 48800.0f;
constexpr float NpcActorHPmax  = 2200.0f;

class PsaGameV1Demo :public GameLogic::INTERFACE_DEVCLASS_GAME {
protected:
    PsagActor::ShaderManager* DemoShaders      = nullptr;
    PsagActor::BricksManager* DemoArchitecture = nullptr;
    PsagActor::ActorsManager* DemoActors       = nullptr;

    PsagActor::PresetScript SYS_PRESET_SC = {};

    // PActor 生命恢复间隔计时器, 开火计时器, 换弹计时器.
    PsagManager::Tools::Timer::GameCycleTimer* PActorHP   = nullptr;
    PsagManager::Tools::Timer::GameCycleTimer* PActorFIRE = nullptr;
    PsagManager::Tools::Timer::GameCycleTimer* PActorCAMM = nullptr;

    PsagManager::Fx::Particle* DemoParticlePActor1 = nullptr; // 回血粒子.
    PsagManager::Fx::Particle* DemoParticlePActor2 = nullptr; // 换弹粒子.
    PsagManager::Fx::Particle* DemoParticleBullet  = nullptr; // 子弹击中粒子.

    PsagManager::Tools::Pawn::GamePlayerPawn*       PlayerPawn   = nullptr;
    PsagManager::Tools::Camera::GamePlayerCameraMP* PlayerCamera = nullptr;

    PsagManager::FxView::TextureViewImage* GuiViewImage = nullptr;

    PsagManager::SceneFinal::GameFinalProcessing* FINAL = nullptr;

    float PawnActorBullet = 128.0f;
    bool  PawnActorChangeAMM = false;

    ResUnique PawnActorUnqiue = NULL;

    void GameInitActorsShader(const Vector2T<uint32_t>& w_size);
    void GameCreatePawnActor();
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