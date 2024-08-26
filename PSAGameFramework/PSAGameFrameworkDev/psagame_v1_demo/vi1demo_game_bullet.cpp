// vi1demo_game_bullet.
#include "v1demo_game.h"

using namespace std;
using namespace PSAG_LOGGER;

void PsaGameV1Demo::GameCreateBulletPawn(const Vector2T<float>& position, float angle) {
    PsagActor::ActorDESC ConfigBulletActor;
    const float BULLET_SPEED = 78.0f;
    
    ConfigBulletActor.ActorPhysicsWorld   = "DemoPhysics";
    ConfigBulletActor.ActorShaderResource = DemoShaders->FindActorShader("BulletActor");

    ConfigBulletActor.InitialPhysics = Vector2T<float>(0.5f, 2.0f);
    ConfigBulletActor.InitialScale   = Vector2T<float>(0.8f, 0.2f);
    ConfigBulletActor.InitialAngle   = angle;

    ConfigBulletActor.InitialPosition =
        PsagManager::Maths::CalcFuncPointAngleDistance(position, angle, 25.0f);

    ConfigBulletActor.InitialSpeed = Vector2T<float>(
            sin(PSAG_M_DEGRAD(-angle - 90.0f)) * BULLET_SPEED, 
            cos(PSAG_M_DEGRAD(-angle - 90.0f)) * BULLET_SPEED
        );

    ConfigBulletActor.InitialVertexColor    = Vector4T(0.0f, 2.0f, 2.0f, 1.0f);
    ConfigBulletActor.CollisionCallbackFunc = ACTOR_COLL_FUNC{

        if (DemoActors->FindGameActor(ThisActor->ActorGetCollision().ActorUniqueCode) != nullptr) {
            auto ActorInfo = DemoActors->FindGameActor(ThisActor->ActorGetCollision().ActorUniqueCode)->ActorGetPrivate();
            
            if (ActorInfo.ActorTypeCode == PsagActorType::ActorTypeAllotter.ActorTypeIs("ActorNpc")) {
                // PawnActor 子弹碰撞 NPC 扣血.
                auto NpcActorOBJ = DemoActors->FindGameActor(ActorInfo.ActorUniqueCode);
                NpcActorOBJ->ActorModifyHealth(
                    0, NpcActorOBJ->ActorGetHealth(0) - PsagManager::Tools::RAND::GenerateRandomFunc(160.0f, 128.0f)
                );
            }
        }
        // 销毁 Bullet => 创建击中粒子.
        DemoActors->DeleteGameActor(ThisActor->ActorGetPrivate().ActorUniqueCode);
        GameCreateParticlesBullet(10.0f, ThisActor->ActorGetPosition());
    };

    // create bullet => actors_manager.
    DemoActors->CreateGameActor(PsagActorType::ActorTypeAllotter.ActorTypeIs("ActorBullet"), ConfigBulletActor);
}

void PsaGameV1Demo::GameCreateBulletNPC(const Vector2T<float>& position, float angle) {
    PsagActor::ActorDESC ConfigBulletActor;
    const float BULLET_SPEED = 52.0f;

    ConfigBulletActor.ActorPhysicsWorld = "DemoPhysics";
    ConfigBulletActor.ActorShaderResource = DemoShaders->FindActorShader("BulletActor");

    ConfigBulletActor.InitialPhysics = Vector2T<float>(10.0f, 5.0f);
    ConfigBulletActor.InitialScale = Vector2T<float>(0.72f, 0.18f);
    ConfigBulletActor.InitialAngle = angle;

    ConfigBulletActor.InitialPosition =
        PsagManager::Maths::CalcFuncPointAngleDistance(position, angle, 22.5f);

    ConfigBulletActor.InitialSpeed = Vector2T<float>(
        sin(PSAG_M_DEGRAD(-angle - 90.0f)) * BULLET_SPEED,
        cos(PSAG_M_DEGRAD(-angle - 90.0f)) * BULLET_SPEED
    );

    ConfigBulletActor.InitialVertexColor = Vector4T(2.0f, 0.2f, 0.0f, 1.0f);
    ConfigBulletActor.CollisionCallbackFunc = ACTOR_COLL_FUNC{

        if (DemoActors->FindGameActor(ThisActor->ActorGetCollision().ActorUniqueCode) != nullptr) {
            auto ActorInfo = DemoActors->FindGameActor(ThisActor->ActorGetCollision().ActorUniqueCode)->ActorGetPrivate();

            if (ActorInfo.ActorTypeCode == PsagActorType::ActorTypeAllotter.ActorTypeIs("ActorNpc")) {
                // NPC 子弹碰撞 NPC 销毁.
                DemoActors->DeleteGameActor(ThisActor->ActorGetPrivate().ActorUniqueCode);
                return;
            }
            if (ActorInfo.ActorTypeCode == PsagActorType::ActorTypeAllotter.ActorTypeIs("ActorPawn")) {
                // NPC 子弹碰撞 PawnActor 扣血.
                auto PawnActorOBJ = DemoActors->FindGameActor(PawnActorUnqiue);
                PawnActorOBJ->ActorModifyHealth(
                    0, PawnActorOBJ->ActorGetHealth(0) - PsagManager::Tools::RAND::GenerateRandomFunc(160.0f, 128.0f)
                );
            }
        }
        // 销毁 Bullet => 创建击中粒子.
        DemoActors->DeleteGameActor(ThisActor->ActorGetPrivate().ActorUniqueCode);
        GameCreateParticlesBullet(10.0f, ThisActor->ActorGetPosition(), 1);
    };

    // create bullet => actors_manager.
    DemoActors->CreateGameActor(PsagActorType::ActorTypeAllotter.ActorTypeIs("ActorBullet"), ConfigBulletActor);
}