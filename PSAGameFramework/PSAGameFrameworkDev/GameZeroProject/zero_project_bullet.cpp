// zero_project_bullet.
#include "zero_project_bullet.h"

using namespace std;
using namespace PSAG_LOGGER;

using CreatePTC = function<void(const Vector2T<float>&, float)>;
class ActorLogicBullet :public PsagActor::ActorLogicBase {
protected:
    PsagManager::Tools::Timer::GameCycleTimer Timer = {};
    CreatePTC ParticleFunc = {};
public:
    ActorLogicBullet(CreatePTC func) :
        ParticleFunc(func) 
    {}
	void ActionLogicRun(PsagActor::Actor* actor_object, float timestep) override {
        if (actor_object->ActorGetLifeTime() > 1.0f)
            actor_object->ActorModifyColorBlend(Vector4T<float>(0.0f, 1.0f, 0.92f, 1.0f));
        // 10 ms create particles fx.
        if (Timer.CycleTimerGetFlag()) {
            ParticleFunc(actor_object->ActorGetPosition(), 3);
            Timer.CycleTimerClearReset(10.0f);
        }
        // bullet actor => mouse_pos => rotate.
        float ActorAngle = PsagManager::Maths::CalcFuncPointsAngle(
            actor_object->ActorMappingWindowCoord(),
            Vector2T<float>(ImGui::GetMousePos().x, ImGui::GetMousePos().y)
        );
        // 相对屏幕鼠标位置施加方向力.
        actor_object->ActorApplyForceMove(Vector2T<float>(
            sin(PSAG_M_DEGRAD(-ActorAngle - 90.0f)) * 10.0f,
            cos(PSAG_M_DEGRAD(-ActorAngle - 90.0f)) * 10.0f
        ));
        actor_object->ActorModifyState(
            actor_object->ActorGetPosition(),
            PsagManager::Maths::CalcDirection(actor_object->ActorGetMoveSpeed())
        );
        auto Refw = actor_object->ActorMappingWindowCoord();

#if PSAG_DEBUG_MODE 
        if (ImPsag::GetDebugGuiFlag()) {
            // draw actor speed.
            ImGui::SetNextWindowPos(ImVec2(Refw.vector_x, Refw.vector_y));
            ImGui::SetNextWindowSize(ImVec2(160.0f, 50.0f));
            ImGuiWindowFlags Flags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs;
            ImGui::Begin(
                to_string(actor_object->ActorGetPrivate().ActorUniqueCode).c_str(),
                (bool*)NULL, Flags
            );
            ImGui::TextColored(
                ImVec4(0.0f, 1.0f, 0.72f, 0.92f),
                "Speed: %.2f", actor_object->ActorGetSpeed()
            );
            ImGui::End();

            ImVec2 BeginPoint = ImPsag::TO_IMVEC2(actor_object->ActorMappingWindowCoord());
            const ImVec4 LineColor(0.58f, 0.58f, 0.58f, 0.24f);
            ImPsag::ListDrawLine(BeginPoint, ImGui::GetMousePos(), LineColor, 1.92f);
        }
#endif
	}
};

void ZPGameBulletSystem::BulletLogic_PPAB(PsagActor::Actor* ThisActor) {
    // bullet collision => free entity.
    ActorEntities.Get()->DeleteGameActor(ThisActor->ActorGetPrivate().ActorUniqueCode);
    CreateParticleFreeB(ThisActor->ActorGetPosition(), 78.0f);
    // 飞行时间 < 1s 忽略.
    if (ThisActor->ActorGetLifeTime() < 1.0f) return;

    auto Entity = GLO_NpcActorsRef->Get()->FindGameActor(
        ThisActor->ActorGetCollision().ActorUniqueCode
    );
    if (Entity != nullptr) {
        Entity->ActorModifyHealth(
            0, Entity->ActorGetHealth(0) - Entity->ActorGetSpeed() * 32.0f
        );
        // HP < 1.0 kill npc_actor entity.
        if (Entity->ActorGetHealth(0) < 1.0f)
            GLO_NpcActorsRef->Get()->DeleteGameActor(Entity->ActorGetPrivate().ActorUniqueCode);
        // global dph counter.
        GLO_PlayerDamageCount += Entity->ActorGetSpeed() * 32.0f;
    }
}

void ZPGameBulletSystem::CreateBullet(const Vector2T<float>& position, float angle) {
    PsagActor::ActorDESC ConfigBulletActor;
    const float BULLET_SPEED = 24.0f;

    ConfigBulletActor.ActorPhysicsWorld = "ZPGamePhysics";
    ConfigBulletActor.ActorShaderResource = ActorShaders.Get()->FindActorShader("BulletActor");

    ConfigBulletActor.InitialPhysics = Vector2T<float>(10.0f, 5.0f);
    ConfigBulletActor.InitialScale   = Vector2T<float>(0.72f, 0.18f);
    ConfigBulletActor.InitialAngle   = angle;

    ConfigBulletActor.InitialPosition =
        PsagManager::Maths::CalcFuncPointAngleDistance(position, angle, 22.5f);

    ConfigBulletActor.InitialSpeed = Vector2T<float>(
        sin(PSAG_M_DEGRAD(-angle - 90.0f)) * BULLET_SPEED,
        cos(PSAG_M_DEGRAD(-angle - 90.0f)) * BULLET_SPEED
    );
    ConfigBulletActor.VectorCalcIsForce = true;
    ConfigBulletActor.VertexColor = Vector4T<float>(1.0f, 0.12f, 0.0f, 1.0f);

    ConfigBulletActor.ActorCollisionThis   = PsagActor::ActorFlagsPhy::ActorPhyGroup2;
    ConfigBulletActor.ActorCollisionFilter = PsagActor::ActorFlagsPhy::ActorPhyGroupALL &
        ~PsagActor::ActorFlagsPhy::ActorPhyGroup2 & 
        ~PsagActor::ActorFlagsPhy::ActorPhyGroup0;

    ActorLogicBullet* LogicObject = new ActorLogicBullet([&](const Vector2T<float>& pos, float num) {
        CreateParticleFly(pos, num);
    });

    ConfigBulletActor.ActorComponentConifg |= PsagActor::ActorFlagsComp::ActorEnableLogic;
    ConfigBulletActor.ActorLogicObject = LogicObject;

    ConfigBulletActor.CollisionCallbackFunc = ACTOR_COLL_FUNC{ BulletLogic_PPAB(ThisActor); };
    
    // create bullet => actors_manager.
    ActorEntities.Get()->CreateGameActor(
        PsagActorType::ActorTypeAllotter.ActorTypeIs("ActorBULLET"), 
        ConfigBulletActor
    );
}

bool ZPGameBulletSystem::LogicInitialization(const Vector2T<uint32_t>& WinSize) {
	// create actor types: bullet
	PsagActorType::ActorTypeAllotter.ActorTypeCreate("ActorBULLET");

	ActorShaders.CreatePointer();
	ActorEntities.CreatePointer();

    PsagManager::SyncLoader::SyncEncDecImage DecodeRawImage;

    auto ImgBullet = PsagManager::SyncLoader::FSLD::EasyFileReadRawData(SYSPATH_REF("zpgame_bullet.png"));
    PsagActor::PresetScript ShaderScript = {};

    PsagActor::ActorShader* RenderBullet = new PsagActor::ActorShader(ShaderScript.TmpScriptDrawImage("Bullet"), WinSize);
    RenderBullet->ShaderImageLADD("Bullet", DecodeRawImage.DecodeImageRawData(ImgBullet));

    ActorShaders.Get()->CreateActorShader("BulletActor", RenderBullet);

    InitParticleSystem(WinSize);

    // 创建 & 注册 ppactor_bullet 全局广播站点.
    BulletStationPPActor.CreatePointer("PPActorBullet");
    GLO_Notify.Get()->RegisterStation(BulletStationPPActor.Get());
	return true;
}

void ZPGameBulletSystem::LogicCloseFree() {
	ActorShaders.DeletePointer();
	ActorEntities.DeletePointer();

    BulletStationPPActor.DeletePointer();
}

bool ZPGameBulletSystem::LogicEventLoopGame(GameLogic::FrameworkParams& RunningState) {
    GLO_PlayerBullets = ActorEntities.Get()->GetSourceData()->size();

    // bullet entities run render.
    ActorEntities.Get()->UpdateManagerData();
    ActorEntities.Get()->RunAllGameActor();
    
    // bullet particle_system run render.
    ParticleSystem.Get()->FxParticleRendering();

    if (BulletStationPPActor.Get()->REC_InfoStatusGet()) {
        PPActorBulletFire FireParams = {};
        // 接收 & 反序列化 射击参数.
        PsagManager::Notify::GameDataSER::PsagDeserialize(
            BulletStationPPActor.Get()->REC_GetInfoRawData(), 
            &FireParams
        );
        CreateBullet(FireParams.Position, FireParams.Angle);
        // end rec state.
        BulletStationPPActor.Get()->REC_InfoStatusOK();
    }
	return true;
}