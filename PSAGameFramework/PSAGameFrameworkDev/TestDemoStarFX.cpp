// TestDemoStarFX.
#include "TestDemoStar.h"

using namespace std;
using namespace PSAG_LOGGER;

StarDemoFX::StarDemoFX(
    PsagActor::ActorsManager* Manager, Vector3T<PsagActor::ActorShader*> Shader,
    const Vector2T<float>& Position, float Angle
) {
    FxActorFireAngle = Angle;
    PsagActor::ActorDESC ConfigStarActor;

    // 卸载碰撞组件, 但是保留控制组件.
    ConfigStarActor.EnableCollision = false;

    ConfigStarActor.ActorPhysicsWorld = "TestPhyWorld";

    ConfigStarActor.InitialPhysics  = Vector2T<float>(1.0f, 1.0f);
    ConfigStarActor.InitialPosition = Position;
    ConfigStarActor.InitialScale    = Vector2T<float>(1.0f, 1.0f);

    ConfigStarActor.ActorShaderResource = Shader.vector_z;

    ConfigStarActor.InitialPosition = Vector2T<float>(
        960.0f * sin((Angle + 90.0f) * (PSAG_M_PI / 180.0f)) + Position.vector_x,
        960.0f * cos((Angle + 90.0f) * (PSAG_M_PI / 180.0f)) + Position.vector_y
    );
    ConfigStarActor.InitialRenderLayer = 51.0f;
    ConfigStarActor.InitialScale = Vector2T<float>(100.0f, 3.2f);
    FxActorUnique[2] = Manager->CreateGameActor(PsagActorType::ActorTypeAllotter.ActorTypeIs("actor_fx"), ConfigStarActor);

    ConfigStarActor.ActorShaderResource = Shader.vector_x;

    ConfigStarActor.InitialRenderLayer    = 52.0f;
    ConfigStarActor.InitialScale    = Vector2T<float>(1.0f, 1.0f);
    ConfigStarActor.InitialPosition = Position;
    FxActorUnique[0] = Manager->CreateGameActor(PsagActorType::ActorTypeAllotter.ActorTypeIs("actor_fx"), ConfigStarActor);

    ConfigStarActor.ActorShaderResource = Shader.vector_y;
    ConfigStarActor.InitialRenderLayer = 54.0f;
    FxActorUnique[1] = Manager->CreateGameActor(PsagActorType::ActorTypeAllotter.ActorTypeIs("actor_fx"), ConfigStarActor);

    ActorShader1 = Shader.vector_x;
    ActorShader2 = Shader.vector_y;
    ActorShader3 = Shader.vector_z;
    ActorManager = Manager;

    PsagLow::PsagSupGraphicsOper::PsagGraphicsImageRawDat DecodeRawImage;
    PsagLow::PsagSupFilesysLoaderBin ParticleTexture("Test/ParticleImgTest.png");

    ActorParticles = new GraphicsEngineParticle::PsagGLEngineParticle(
        Vector2T<uint32_t>(), 
        DecodeRawImage.DecodeImageRawData(ParticleTexture.GetDataBinary())
    );
    ActorParticles->SetParticleTwisted(2.0f);
}

void StarDemoFX::StarDemoFxRender() {
    auto FxActorObj1 = ActorManager->FindGameActor(FxActorUnique[0]);
    auto FxActorObj2 = ActorManager->FindGameActor(FxActorUnique[1]);

    // 生成聚合粒子.
    if (FxActorTimer > 1.2f) {
        GraphicsEngineParticle::ParticleGenerator CreatePartc;
        Vector2T<float> ColorSystem = Vector2T<float>(0.05f * FxActorAnimTimer, 0.15f * FxActorAnimTimer);

        CreatePartc.ConfigCreateMode(GraphicsEngineParticle::ParticlesGenMode::PrtcPoly);
        CreatePartc.ConfigCreateNumber(FxActorAnimTimer * FxActorAnimTimer * 0.5f + 8.0f);
        CreatePartc.ConfigLifeDispersion(Vector2T<float>(35.0f, 70.0f));
        CreatePartc.ConfigSizeDispersion(Vector2T<float>(0.1f * FxActorAnimTimer, 0.25f * FxActorAnimTimer));

        // particles color rand: red,blue.
        CreatePartc.ConfigRandomColorSystem(ColorSystem, ColorSystem, ColorSystem, GraphicsEngineParticle::ParticlesGenMode::ChannelsGB);
        CreatePartc.ConfigRandomDispersion(
            Vector2T<float>(0.72f, 1.0f),
            Vector2T<float>(-28.0f * FxActorAnimTimer, 28.0f * FxActorAnimTimer),
            Vector3T<float>(FxActorObj1->ActorGetPosition().vector_x, FxActorObj1->ActorGetPosition().vector_y, 0.0f)
        );
        ActorParticles->ParticleCreate(&CreatePartc);

        // clear cycles_timer.
        FxActorTimer = 0.0f;
    }

    ActorShader1->UniformSetContext([&]() { ActorShader1->UniformFP32("Brightness", FxActorAnimTimer * 0.5f); });
    FxActorObj1->ActorModifyScale(Vector2T<float>(FxActorAnimTimer, FxActorAnimTimer));
    FxActorObj2->ActorModifyScale(Vector2T<float>(FxActorAnimTimer * 1.7f, FxActorAnimTimer * 1.7f));
    
    ActorParticles->UpdateParticleData();
    ActorParticles->RenderParticleFX();

    FxActorAnimTimer += 0.002f;
    FxActorTimer += 0.02f;
    if (FxActorAnimTimer > 20.0f) FxActorAnimTimer = 20.0f;
}

void StarDemoFX::StarDemoFxFire() {
    if (FxActorAnimTimer > 19.9f) {
        FxActorFireFlag = true;
        ActorShader3->UniformSetContext([&]() { ActorShader3->UniformInt32("FireFlag", 1); });

        auto FxActorObj3 = ActorManager->FindGameActor(FxActorUnique[2]);
        FxActorObj3->ActorModifyState(FxActorObj3->ActorGetPosition(), FxActorFireAngle);
    }
}

Vector2T<float> StarDemoFX::StarDemoFxCameraOffset(float* Scale, float* Rotate) {
    FxActorCameraTimer += 0.32f;
    *Scale += FxActorAnimTimer * 0.16f;
    *Rotate = sin(FxActorAnimTimer) * 20.0f;
    return Vector2T<float>(
        sin(FxActorCameraTimer) * (12.0f + FxActorAnimTimer * 0.5f),
        cos(FxActorCameraTimer * 1.2024f) * (12.0f + FxActorAnimTimer * 0.5f)
    );
}

StarDemoFX::~StarDemoFX() {
    ActorManager->DeleteGameActor(FxActorUnique[0]);
    ActorManager->DeleteGameActor(FxActorUnique[1]);
}