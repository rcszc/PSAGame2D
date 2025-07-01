// zero_project_bullet. RCSZ.
// game module(desc): gun bullet.

#ifndef __ZERO_PROJECT_BULLET
#define __ZERO_PROJECT_BULLET
#include "base_define_header.hpp"

class BulletParticle {
protected:
    Mptr::DefinePointer<PsagManager::Fx::Particle> ParticleSystem = {};
    void InitParticleSystem(const Vector2T<uint32_t>& window_size);

    void CreateParticleFly  (const Vector2T<float>& position, float number);
    void CreateParticleGuide(const Vector2T<float>& position);

    void CreateParticleFree0(const Vector2T<float>& position, float number);
    void CreateParticleFree1(const Vector2T<float>& position);

    void DebugParticleSystem();
public:
    ~BulletParticle();
};

class ZPGameBulletSystem :public GameLogic::INTERFACE_DEVCLASS_GAME,
    public BulletParticle, public ZPGameGlobale {
protected:
    Mptr::DefinePointer<PsagActor::ShaderManager> ActorShaders  = {};
    Mptr::DefinePointer<PsagActor::ActorsManager> SharpEntities = {};
    Mptr::DefinePointer<PsagActor::ActorsManager> GuideEntities = {};

    Mptr::DefinePointer<PsagManager::Notify::StationSystem> BulletStationPPActor = {};

    void SharpBullet_PPAB(PsagActor::Actor* ThisActor);
    void GuideBullet_PPAB(PsagActor::Actor* ThisActor);

    void CreateSharpBullet(const Vector2T<float>& pos, float angle);
    void CreateGuideBullet(const Vector2T<float>& pos, float angle);
public:
    // init: return flag: false:failed, true:success.
    bool LogicInitialization(const Vector2T<uint32_t>& WinSize);
    void LogicCloseFree();
    // loop: return flag: false:continue, true:exit.
    bool LogicEventLoopGame(GameLogic::FrameworkParams& RunningState);
};

#endif