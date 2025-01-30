// special_project. RCSZ.
// new_year_fireworks. 2025_01_27.

#ifndef __SPECIAL_PROJECT_H
#define __SPECIAL_PROJECT_H
#include "../../../PSAGameFrameworkCore/framework_header_actor.h"
#include "../../../PSAGameFrameworkCore/framework_header_manager.h"

inline std::string SPE_SYSPATH_REF(const std::string& name) {
    return "PSAGameFrameworkDev/GameZeroProject/NewYearFireworks/Material/" + name;
}
namespace Mptr = PsagManager::PtrMAG;
namespace LOADFILE = PsagManager::SyncLoader::FSLD;

class FireworksParticle {
protected:
    Mptr::DefinePointer<PsagManager::Fx::Particle> ParticleSystem = {};
    void InitParticleSystem(const Vector2T<uint32_t>& window_size);

    void CreateParticleFly  (const Vector2T<float>& position, float number);
    void CreateParticleFreeA(const Vector2T<float>& position, float number);
    void CreateParticleFreeB(const Vector2T<float>& position, float number);
public:
    ~FireworksParticle();
};

class FireworksSystem :public GameLogic::INTERFACE_DEVCLASS_GAME,
    public FireworksParticle {
protected:
    Mptr::DefinePointer<PsagActor::ShaderManager> ActorShaders  = {};
    Mptr::DefinePointer<PsagActor::ActorsManager> ActorEntities = {};

    Mptr::DefinePointer<PsagManager::Tools::Sound::GamePlayerSound>
        MainMusicPlayer = {};
    Mptr::DefinePointer<PsagManager::Tools::Timer::GameCycleTimer>
        FireCyclesTimer = {};

    bool MUSIC_FLAG = false, FIRE_FLAG = false;
    void CreateFireworks(
        const Vector2T<float>& position, const Vector2T<float>& speed, 
        float flytime
    );
    void CreateBaseFireworks(const Vector2T<float>& position, float flytime);
public:
    // init: return flag: false:failed, true:success.
    bool LogicInitialization(const Vector2T<uint32_t>& WinSize);
    void LogicCloseFree();
    // loop: return flag: false:continue, true:exit.
    bool LogicEventLoopGame(GameLogic::FrameworkParams& RunningState);
};

#endif