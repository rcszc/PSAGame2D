// zero_project_bullet.
#include "zero_project_bullet.h"

using namespace std;
using namespace PSAG_LOGGER;

using CreateFX = function<void(const Vector2T<float>&, float)>;
class BulletBase {
protected:
    CreateFX GenParticleFunc = {};
    PsagManager::Tools::Timer::GameCycleTimer Timer = {};
    Vector2T<float> ThisPosition = {};

    void DrawDebugValues(PsagActor::Actor* actor_object) {
#if PSAG_DEBUG_MODE 
        if (ImPsag::GetDebugGuiFlag()) {
            auto Refw = actor_object->ActorMappingWindowCoord();
            // draw actor speed.
            ImGui::SetNextWindowPos(ImVec2(Refw.vector_x, Refw.vector_y));
            ImGui::SetNextWindowSize(ImVec2(160.0f, 50.0f));
            ImGuiWindowFlags Flags = ImGuiWindowFlags_NoBackground |
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs;
            ImGui::Begin(
                to_string(actor_object->ActorGetPrivate().ActorUniqueCode).c_str(),
                (bool*)NULL, Flags
            );
            ImGui::TextColored(
                ImVec4(0.0f, 1.0f, 0.72f, 0.92f),
                "Speed: %.2f", actor_object->ActorGetSpeed()
            );
            ImGui::End();
            // mouse => bullet actor, draw line.
            ImVec2 BeginPoint = ImPsag::TO_IMVEC2(actor_object->ActorMappingWindowCoord());
            const ImVec4 LineColor(0.58f, 0.58f, 0.58f, 0.24f);
            ImPsag::ListDrawLine(BeginPoint, ImGui::GetMousePos(), LineColor, 1.92f);
        }
#endif
    }
    void GenerateFX(PsagActor::Actor* actor_object) {
        if (Timer.CycleTimerGetFlag()) {
            // 12 ms create particles fx.
            Timer.CycleTimerClearReset(12.0f);
            GenParticleFunc(ThisPosition, 2);
        }
        ThisPosition = actor_object->ActorGetPosition();
    }
};

class ActorSharpBullet :public BulletBase,
    public PsagActor::ActorLogicBase
{
public:
    ActorSharpBullet(CreateFX FUNC) {
        GenParticleFunc = FUNC;
    }
    void ActionLogicRun(PsagActor::Actor* actor_object, float timestep) override {
        GenerateFX(actor_object);
        DrawDebugValues(actor_object);
    }
};

class ActorGuideBullet :public BulletBase, 
    public PsagActor::ActorLogicBase 
{
public:
    ActorGuideBullet(CreateFX FUNC) {
        GenParticleFunc = FUNC;
    }
	void ActionLogicRun(PsagActor::Actor* actor_object, float timestep) override {
        GenerateFX(actor_object);
        DrawDebugValues(actor_object);
	}
};

void ZPGameBulletSystem::SharpBullet_PPAB(PsagActor::Actor* ThisActor) {
    // bullet collision => free entity.
    SharpEntities.Get()->DeleteGameActor(ThisActor->ActorGetPrivate().ActorUniqueCode);
    CreateParticleFree1(ThisActor->ActorGetPosition());

    auto Entity = GLO_NpcActorsRef->Get()->FindGameActor(
        ThisActor->ActorGetCollision().ActorUniqueCode
    );
    if (Entity != nullptr) {
        Entity->ActorModifyHealth(0, Entity->ActorGetHealth(0) - 128.0f);
        // HP < 1.0 kill npc_actor entity.
        if (Entity->ActorGetHealth(0) < 1.0f)
            GLO_NpcActorsRef->Get()->DeleteGameActor(Entity->ActorGetPrivate().ActorUniqueCode);
        // global dph counter.
        GLO_PlayerDamageCount += 128.0f;
    }
}

void ZPGameBulletSystem::GuideBullet_PPAB(PsagActor::Actor* ThisActor) {
    // bullet collision => free entity.
    GuideEntities.Get()->DeleteGameActor(ThisActor->ActorGetPrivate().ActorUniqueCode);
    CreateParticleFree0(ThisActor->ActorGetPosition(), 128.0f);
    // 飞行时间 < 1s 忽略.
    if (ThisActor->ActorGetLifeTime() < 1.0f) 
        return;
    auto Entity = GuideEntities.Get()->FindGameActor(
        ThisActor->ActorGetCollision().ActorUniqueCode
    );
    if (Entity != nullptr) {
        Entity->ActorModifyHealth(
            0, Entity->ActorGetHealth(0) - Entity->ActorGetSpeed() * 58.0f
        );
        // HP < 1.0 kill npc_actor entity.
        if (Entity->ActorGetHealth(0) < 1.0f)
            GuideEntities.Get()->DeleteGameActor(Entity->ActorGetPrivate().ActorUniqueCode);
        // global dph counter.
        GLO_PlayerDamageCount += Entity->ActorGetSpeed() * 58.0f;
    }
}

void ZPGameBulletSystem::CreateSharpBullet(const Vector2T<float>& pos, float angle) {
    PsagActor::ActorDESC ConfigBulletActor;
    const float BULLET_SPEED = 24.0f;

    Vector2T<float> Posotion = 
        PsagManager::Maths::CalcFuncPointAngleDistance(pos, angle, 22.5f);

    ConfigBulletActor.ActorPhysicsWorld = "ZPGamePhysics";
    ConfigBulletActor.ActorShaderResource = ActorShaders.Get()->FindActorShader("ShaderSBULL");

    ConfigBulletActor.InitialPhysics  = Vector2T<float>(10.0f, 5.0f);
    ConfigBulletActor.InitialScale    = Vector2T<float>(0.58f, 0.12f);
    ConfigBulletActor.InitialAngle    = angle;
    ConfigBulletActor.InitialPosition = Posotion;

    ConfigBulletActor.InitialSpeed = Vector2T<float>(
        sin(PSAG_M_DEGRAD(-angle - 90.0f)) * BULLET_SPEED,
        cos(PSAG_M_DEGRAD(-angle - 90.0f)) * BULLET_SPEED
    );
    ConfigBulletActor.VectorCalcIsForce = true;
    ConfigBulletActor.VertexColor = Vector4T<float>(0.0f, 1.0f, 1.0f, 1.0f);

    ConfigBulletActor.ActorCollisionThis   = PsagActor::ActorFlagsPhy::ActorPhyGroup2;
    ConfigBulletActor.ActorCollisionFilter = PsagActor::ActorFlagsPhy::ActorPhyGroupALL &
        ~PsagActor::ActorFlagsPhy::ActorPhyGroup2 &
        ~PsagActor::ActorFlagsPhy::ActorPhyGroup0;

    ActorSharpBullet* LogicObject = new ActorSharpBullet(
        [&](const Vector2T<float>& pos, float num) {
            CreateParticleFly(pos, num);
        });
    ConfigBulletActor.ActorComponentConifg |= PsagActor::ActorFlagsComp::ActorEnableLogic;
    ConfigBulletActor.ActorLogicObject = LogicObject;
    ConfigBulletActor.CollisionCallbackFunc = ACTOR_COLL_FUNC{ SharpBullet_PPAB(ThisActor); };

    // create bullet => actors_manager.
    SharpEntities.Get()->CreateGameActor(
        PsagActorType::ActorTypeAllotter.ActorTypeIs("GuideBullet"),
        ConfigBulletActor
    );
}

void ZPGameBulletSystem::CreateGuideBullet(const Vector2T<float>& pos, float angle) {
    if (GuideEntities.Get()->GetSourceData()->size() > 10)
        return;
    PsagActor::ActorDESC ConfigBulletActor;
    const float BULLET_SPEED = 24.0f;

    Vector2T<float> Posotion = Vector2T<float>(
        PsagManager::Tools::Random::Func::GenerateRandomFuncFP32(-100.0f, 100.0f) + pos.vector_x,
        PsagManager::Tools::Random::Func::GenerateRandomFuncFP32(-100.0f, 100.0f) + pos.vector_y
    );

    ConfigBulletActor.ActorPhysicsWorld = "ZPGamePhysics";
    ConfigBulletActor.ActorShaderResource = ActorShaders.Get()->FindActorShader("ShaderGBULL");

    ConfigBulletActor.InitialPhysics  = Vector2T<float>(10.0f, 5.0f);
    ConfigBulletActor.InitialScale    = Vector2T<float>(0.72f, 0.18f);
    ConfigBulletActor.InitialAngle    = angle;
    ConfigBulletActor.InitialPosition = Posotion;

    ConfigBulletActor.InitialSpeed = Vector2T<float>(
        sin(PSAG_M_DEGRAD(-angle - 90.0f)) * BULLET_SPEED,
        cos(PSAG_M_DEGRAD(-angle - 90.0f)) * BULLET_SPEED
    );
    ConfigBulletActor.VectorCalcIsForce = true;
    ConfigBulletActor.VertexColor = Vector4T<float>(0.0f, 1.0f, 1.0f, 1.0f);

    ConfigBulletActor.ActorCollisionThis   = PsagActor::ActorFlagsPhy::ActorPhyGroup2;
    ConfigBulletActor.ActorCollisionFilter = PsagActor::ActorFlagsPhy::ActorPhyGroupALL &
        ~PsagActor::ActorFlagsPhy::ActorPhyGroup2 & 
        ~PsagActor::ActorFlagsPhy::ActorPhyGroup0;

    ActorGuideBullet* LogicObject = new ActorGuideBullet(
        [&](const Vector2T<float>& pos, float num) {
        CreateParticleFly(pos, num);
    });
    ConfigBulletActor.ActorComponentConifg |= PsagActor::ActorFlagsComp::ActorEnableLogic;
    ConfigBulletActor.ActorLogicObject = LogicObject;
    ConfigBulletActor.CollisionCallbackFunc = ACTOR_COLL_FUNC{ GuideBullet_PPAB(ThisActor); };
    
    // create bullet => actors_manager.
    GuideEntities.Get()->CreateGameActor(
        PsagActorType::ActorTypeAllotter.ActorTypeIs("GuideBullet"), 
        ConfigBulletActor
    );
    CreateParticleGuide(Posotion);
}

bool ZPGameBulletSystem::LogicInitialization(const Vector2T<uint32_t>& WinSize) {
	// create actor types: bullet
	PsagActorType::ActorTypeAllotter.ActorTypeCreate("GuideBullet");

	ActorShaders.CreatePointer();
	SharpEntities.CreatePointer();
    GuideEntities.CreatePointer();

    PsagManager::SyncLoader::SyncEncDecImage DecodeRawImage;
    PsagActor::PresetScript ShaderScript = {};

    auto SharpBulletImg = 
        PsagManager::SyncLoader::FSLD::EasyFileReadRawData(SYSPATH_REF("zpgame_bullet_b.png"));
    auto GuideBulletImg = 
        PsagManager::SyncLoader::FSLD::EasyFileReadRawData(SYSPATH_REF("zpgame_bullet_a.png"));

    PsagActor::ActorShader* RenderSharpBULL =
        new PsagActor::ActorShader(ShaderScript.TmpScriptDrawImage("SharpBullet"), WinSize);
    PsagActor::ActorShader* RenderGuideBULL =
        new PsagActor::ActorShader(ShaderScript.TmpScriptDrawImage("GuideBullet"), WinSize);

    RenderSharpBULL->ShaderImageLADD("SharpBullet", 
        DecodeRawImage.DecodeImageRawData(SharpBulletImg)
    );
    RenderGuideBULL->ShaderImageLADD("GuideBullet",
        DecodeRawImage.DecodeImageRawData(GuideBulletImg)
    );
    ActorShaders.Get()->CreateActorShader("ShaderSBULL", RenderSharpBULL);
    ActorShaders.Get()->CreateActorShader("ShaderGBULL", RenderGuideBULL);

    InitParticleSystem(WinSize);

    // 创建 & 注册 ppactor_bullet 全局广播站点.
    BulletStationPPActor.CreatePointer("PPActorBullet");
    GLO_Notify.Get()->RegisterStation(BulletStationPPActor.Get());
	return true;
}

void ZPGameBulletSystem::LogicCloseFree() {
	ActorShaders.DeletePointer();
	SharpEntities.DeletePointer();
    GuideEntities.DeletePointer();
    BulletStationPPActor.DeletePointer();
}

bool ZPGameBulletSystem::LogicEventLoopGame(GameLogic::FrameworkParams& RunningState) {
    GLO_PlayerBullets = SharpEntities.Get()->GetSourceData()->size();

    // bullet entities run render.
    SharpEntities.Get()->UpdateManagerData();
    SharpEntities.Get()->RunAllGameActor();

    GuideEntities.Get()->UpdateManagerData();
    GuideEntities.Get()->RunAllGameActor();
    
    // bullet particle_system run render.
    ParticleSystem.Get()->FxParticleRendering();
    
    if (BulletStationPPActor.Get()->REC_InfoStatusGet()) {
        PPActorBulletFire FireParams = {};
        // 接收 & 反序列化 射击参数.
        PsagManager::Notify::GameDataSER::PsagDeserialize(
            BulletStationPPActor.Get()->REC_GetInfoRawData(), 
            &FireParams
        );
        switch(FireParams.FireBulletType){
        case(0): CreateSharpBullet(FireParams.Position, FireParams.Angle); break;
        case(1):
            for (size_t i = 0; i < 5 - GLO_PlayerBullets; ++i)
                CreateGuideBullet(FireParams.Position, FireParams.Angle);
            break;
        }
        // end rec state.
        BulletStationPPActor.Get()->REC_InfoStatusOK();
    }
    // run debug draw frame.
    DebugParticleSystem();
	return true;
}