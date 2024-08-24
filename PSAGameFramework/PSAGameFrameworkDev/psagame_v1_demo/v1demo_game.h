// v1demo_game. 2024_08_022 RCSZ.
// Framework DemoV1.

#ifndef __V1DEMO_GAME_H
#define __V1DEMO_GAME_H
#include "../../PSAGameFrameworkCore/framework_dev_actor.h"
#include "../../PSAGameFrameworkCore/framework_dev_manager.h"

extern const char* PawnActorFrag;
extern const char* BulletActorFrag;

class PsaGameV1Demo :public GameLogic::INTERFACE_DEVCLASS_GAME {
protected:
    PsagActor::DebugTools::DebugWindowGuiFPS* FPS_TEST = nullptr;

    PsagActor::ShaderManager* DemoShaders      = nullptr;
    PsagActor::BricksManager* DemoArchitecture = nullptr;
    PsagActor::ActorsManager* DemoActors       = nullptr;

    PsagActor::PresetScript SYS_PRESET_SC = {};

    // PActor 生命恢复间隔计时器, 开火计时器.
    PsagManager::Tools::Timer::GameCycleTimer* PActorHP   = nullptr;
    PsagManager::Tools::Timer::GameCycleTimer* PActorFIRE = nullptr;

    PsagManager::Fx::Particle* DemoParticlePActor = nullptr;

    PsagManager::Tools::Pawn::GamePlayerPawn*        PlayerPawn   = nullptr;
    PsagManager::Tools::Camera::GamePlayerComaeraMP* PlayerCamera = nullptr;

    ResUnique PawnActorUnqiue = NULL;

    float PawnActorHPmax  = 3200.0f;
    float PawnActorAngle  = 0.0f;
    float PawnActorBullet = 320.0f;

    float NpcActorHPmax = 2200.0f;

    void GameInitPawnActor();
    void GameInitBoundary();
    void GameInitParticleSystem(const Vector2T<uint32_t>& w_size);

    void GameCreateBullet(const Vector2T<float>& position, float angle);
    void GameCreateNPC(const Vector2T<float>& position);
    void GameCreateParticlesPActor(float number);

public:
    // init: return flag: false:failed, true:success.
    bool LogicInitialization(const Vector2T<uint32_t>& WinSize);
    void LogicCloseFree();
    // loop: return flag: false:continue, true:exit.
    bool LogicEventLoopGame(GameLogic::FrameworkParams& RunningState);
    bool LogicEventLoopGui(GameLogic::FrameworkParams& RunningState);
};

#endif