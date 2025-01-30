// zero_project_bullet. RCSZ.
// game module(desc): gun bullet.

#ifndef __ZERO_PROJECT_BULLET
#define __ZERO_PROJECT_BULLET
#include "base_define_header.hpp"

class BulletParticle {
protected:
    Mptr::DefinePointer<PsagManager::Fx::Particle> ParticleSystem = {};
    void InitParticleSystem(const Vector2T<uint32_t>& window_size);

    void CreateParticleFly (const Vector2T<float>& position, float number);
    void CreateParticleFreeB(const Vector2T<float>& position, float number);
public:
    ~BulletParticle();
};

class ZPGameBulletSystem :public GameLogic::INTERFACE_DEVCLASS_GAME,
    public BulletParticle, public ZPGameGlobale {
protected:
    Mptr::DefinePointer<PsagActor::ShaderManager> ActorShaders  = {};
    Mptr::DefinePointer<PsagActor::ActorsManager> ActorEntities = {};

    Mptr::DefinePointer<PsagManager::Notify::StationSystem> BulletStationPPActor = {};

    void BulletLogic_PPAB(PsagActor::Actor* ThisActor);

    void CreateBullet(const Vector2T<float>& position, float angle);
public:
    // init: return flag: false:failed, true:success.
    bool LogicInitialization(const Vector2T<uint32_t>& WinSize);
    void LogicCloseFree();
    // loop: return flag: false:continue, true:exit.
    bool LogicEventLoopGame(GameLogic::FrameworkParams& RunningState);
};

#endif