// vi1demo_game_bullet.
#include "v1demo_game.h"

using namespace std;
using namespace PSAG_LOGGER;

void PsaGameV1Demo::GameCreateBullet(const Vector2T<float>& position, float angle) {
    PsagActor::ActorDESC ConfigBulletActor;
    const float BULLET_SPEED = 70.0f;
    
    ConfigBulletActor.ActorPhysicsWorld   = "DemoPhysics";
    ConfigBulletActor.ActorShaderResource = DemoShaders->FindActorShader("BulletActor");

    ConfigBulletActor.InitialPhysics = Vector2T<float>(5.0f, 2.0f);
    ConfigBulletActor.InitialScale   = Vector2T<float>(0.76f, 0.18f);
    ConfigBulletActor.InitialAngle   = angle;

    ConfigBulletActor.InitialPosition =
        PsagManager::Maths::CalcFuncPointAngleDistance(position, angle, 22.5f);

    ConfigBulletActor.InitialSpeed = Vector2T<float>(
            sin(PSAG_M_DEGRAD(-angle - 90.0f)) * BULLET_SPEED, 
            cos(PSAG_M_DEGRAD(-angle - 90.0f)) * BULLET_SPEED
        );

    ConfigBulletActor.CollisionCallbackFunc = ACTOR_COLL_FUNC{
        DemoActors->DeleteGameActor(ThisActor->ActorGetPrivate().ActorUniqueCode);
    };
    // create bullet => actors_manager.
    DemoActors->CreateGameActor(PsagActorType::ActorTypeAllotter.ActorTypeIs("ActorBullet"), ConfigBulletActor);
}