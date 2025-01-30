// special_project_init.
#include "special_project.h"

using namespace std;
using namespace PSAG_LOGGER;

using ManagerRef = Mptr::DefinePointer<PsagActor::ActorsManager>*;
using CreateParticle = function<void(const Vector2T<float>&, float)>;

class FActorLogic :public PsagActor::ActorLogicBase {
protected:
    CreateParticle ParticleFunc[2] = {};
    ManagerRef ObjectRef = nullptr;
    float FlyTimeValue = 0.0f;

    PsagManager::Tools::Timer::GameCycleTimer Timer = {};
public:
    FActorLogic(CreateParticle func0, CreateParticle func1, ManagerRef ptr, float flytime) :
        ParticleFunc{ func0,func1 }, ObjectRef(ptr), FlyTimeValue(flytime)
    {}
    void ActionLogicRun(PsagActor::Actor* actor_object, float timestep) override {
        // 10 ms create particles fx.
        if (Timer.CycleTimerGetFlag()) {
            ParticleFunc[0](actor_object->ActorGetPosition(), 2);
            Timer.CycleTimerClearReset(30.0f);
        }
        if (actor_object->ActorGetLifeTime() > FlyTimeValue) {
            ParticleFunc[1](actor_object->ActorGetPosition(), 100);
            ObjectRef->Get()->DeleteGameActor(actor_object->ActorGetPrivate().ActorUniqueCode);
        }
    }
};

void FireworksSystem::CreateFireworks(
    const Vector2T<float>& position, const Vector2T<float>& speed, float flytime
) {
    PsagActor::ActorDESC ConfigActor;

    ConfigActor.ActorPhysicsWorld   = "ZPGamePhysics";
    ConfigActor.ActorShaderResource = ActorShaders.Get()->FindActorShader("BASE");

    ConfigActor.InitialPhysics    = Vector2T<float>(10.0f, 5.0f);
    ConfigActor.InitialScale      = Vector2T<float>(0.58f, 0.58f);
    ConfigActor.InitialAngle      = 0.0f;
    ConfigActor.InitialAngleSpeed = 
        PsagManager::Tools::Random::Func::GenerateRandomFuncFP32(10.0f, 32.0f);
    ConfigActor.InitialSpeed      = speed;
    ConfigActor.InitialPosition   = position;

    ConfigActor.VectorCalcIsForce = false;
    ConfigActor.VertexColor = Vector4T<float>(2.0f, 2.0f, 2.0f, 2.0f);

    ConfigActor.ActorCollisionThis   = PsagActor::ActorFlagsPhy::ActorPhyGroup10;
    ConfigActor.ActorCollisionFilter = PsagActor::ActorFlagsPhy(NULL);

    FActorLogic* LogicObject = new FActorLogic(
        [&](const Vector2T<float>& pos, float num) { CreateParticleFly (pos, num); }, 
        [&](const Vector2T<float>& pos, float num) { CreateParticleFreeB(pos, num); },
        &ActorEntities, flytime
    );
    ConfigActor.ActorComponentConifg |= PsagActor::ActorFlagsComp::ActorEnableLogic;
    ConfigActor.ActorLogicObject = LogicObject;

    // create actor => actors_manager.
    ActorEntities.Get()->CreateGameActor(
        PsagActorType::ActorTypeAllotter.ActorTypeIs("ActorFIRE"),
        ConfigActor
    );
}

using CreateChild = function<void(const Vector2T<float>&, const Vector2T<float>&, float)>;

class BFActorLogic :public PsagActor::ActorLogicBase {
protected:
    CreateParticle ParticleFuncBase[2] = {};
    CreateChild    ParticleFuncChild   = {};

    ManagerRef ObjectRef = nullptr;
    float FlyTimeValue = 0.0f;

    PsagManager::Tools::Timer::GameCycleTimer Timer = {};
public:
    BFActorLogic(CreateChild func0, CreateParticle func1, CreateParticle func2, ManagerRef ptr, float flytime) :
        ParticleFuncChild(func0), ParticleFuncBase{ func1,func2 }, ObjectRef(ptr), FlyTimeValue(flytime)
    {}
    void ActionLogicRun(PsagActor::Actor* actor_object, float timestep) override {
        // 10 ms create particles fx.
        if (Timer.CycleTimerGetFlag()) {
            ParticleFuncBase[0](actor_object->ActorGetPosition(), 10);
            Timer.CycleTimerClearReset(10.0f);
        }
        if (actor_object->ActorGetLifeTime() > FlyTimeValue) {
            for (size_t i = 0; i < 16; ++i) {
                ParticleFuncChild(
                    actor_object->ActorGetPosition(),
                    Vector2T<float>(
                        PsagManager::Tools::Random::Func::GenerateRandomFuncFP32(-5.8f, 5.8f),
                        PsagManager::Tools::Random::Func::GenerateRandomFuncFP32(-5.8f, 5.8f)
                    ),  
                    PsagManager::Tools::Random::Func::GenerateRandomFuncFP32(2.0f, 3.0f)
                );
            }
            ParticleFuncBase[1](actor_object->ActorGetPosition(), 160.0f);
            ObjectRef->Get()->DeleteGameActor(
                actor_object->ActorGetPrivate().ActorUniqueCode);
        }
    }
};

void FireworksSystem::CreateBaseFireworks(const Vector2T<float>& position, float flytime) {
    PsagActor::ActorDESC ConfigActor;

    ConfigActor.ActorPhysicsWorld = "ZPGamePhysics";
    ConfigActor.ActorShaderResource = ActorShaders.Get()->FindActorShader("BASE");

    ConfigActor.InitialPhysics  = Vector2T<float>(10.0f, 5.0f);
    ConfigActor.InitialScale    = Vector2T<float>(1.6f, 1.6f);
    ConfigActor.InitialAngle    = 0.0f;
    ConfigActor.InitialAngleSpeed =
        PsagManager::Tools::Random::Func::GenerateRandomFuncFP32(200.0f, 800.0f);
    ConfigActor.InitialSpeed    = Vector2T<float>(0.0f, 5.8f);
    ConfigActor.InitialPosition = position;

    ConfigActor.VectorCalcIsForce = true;
    ConfigActor.VertexColor = Vector4T<float>(2.0f, 2.0f, 2.0f, 2.0f);

    ConfigActor.ActorCollisionThis = PsagActor::ActorFlagsPhy::ActorPhyGroup10;
    ConfigActor.ActorCollisionFilter = PsagActor::ActorFlagsPhy(NULL);

    BFActorLogic* LogicObject = new BFActorLogic(
        [&](const Vector2T<float>& pos, const Vector2T<float>&  vec, float num) 
        { CreateFireworks(pos, vec, num); },
        [&](const Vector2T<float>& pos, float num) { CreateParticleFly(pos, num); },
        [&](const Vector2T<float>& pos, float num) { CreateParticleFreeA(pos, num); },
        &ActorEntities, flytime
    );
    ConfigActor.ActorComponentConifg |= PsagActor::ActorFlagsComp::ActorEnableLogic;
    ConfigActor.ActorLogicObject = LogicObject;

    // create actor => actors_manager.
    ActorEntities.Get()->CreateGameActor(
        PsagActorType::ActorTypeAllotter.ActorTypeIs("ActorFIRE"),
        ConfigActor
    );
}