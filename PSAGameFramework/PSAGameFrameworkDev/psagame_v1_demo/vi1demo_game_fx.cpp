// vi1demo_game_fx.
#include "v1demo_game.h"

using namespace std;
using namespace PSAG_LOGGER;

void PsaGameV1Demo::GameInitParticleSystem(const Vector2T<uint32_t>& w_size) {
	PsagManager::SyncLoader::SyncDecodeImage DecodeRawImage;
	PsagManager::Fx::ParticleDESC CreatePActorParticle = {};

	PsagManager::SyncLoader::SyncBinFileLoad PActorImage1("demo_v1_material/psag_v1_particle2.png");
	PsagManager::SyncLoader::SyncBinFileLoad PActorImage2("demo_v1_material/psag_v1_particle1.png");
	PsagManager::SyncLoader::SyncBinFileLoad PActorImage3("demo_v1_material/psag_v1_particle0.png");

	CreatePActorParticle.ParticleRenderResolution = w_size;
	CreatePActorParticle.ParticlesDisturbance     = 0.0f;
	CreatePActorParticle.ParticleRenderTexture    = DecodeRawImage.DecodeImageRawData(PActorImage1.GetDataBinary());

	DemoParticlePActor1 = new PsagManager::Fx::Particle(CreatePActorParticle);

	// 复用 particle DESC.
	CreatePActorParticle.ParticleRenderTexture = DecodeRawImage.DecodeImageRawData(PActorImage2.GetDataBinary());
	DemoParticlePActor2 = new PsagManager::Fx::Particle(CreatePActorParticle);

	// 复用 particle DESC.
	CreatePActorParticle.ParticleRenderTexture = DecodeRawImage.DecodeImageRawData(PActorImage3.GetDataBinary());
	DemoParticleBullet = new PsagManager::Fx::Particle(CreatePActorParticle);
}

void PsaGameV1Demo::GameCreateParticlesPActorADHP(float number) {
	PsagManager::Fx::ParticleGenDESC CreateParticles;

	CreateParticles.PariclesNumber = number;
	CreateParticles.ParticlesColorMode  = PsagManager::Fx::GenMode::ChannelsRGB;
	CreateParticles.ParticlesLaunchMode = PsagManager::Fx::GenMode::PrtcPoints;

	CreateParticles.ParticlesCgRandom   = Vector2T<float>(0.32f, 1.0f);
	CreateParticles.ParticlesLifeRandom = Vector2T<float>(150.0f, 250.0f);
	CreateParticles.ParticlesSizeRandom = Vector2T<float>(1.8f, 4.8f);
	CreateParticles.ParticlesPosRandom  = Vector2T<float>(-38.0f, 38.0f);
	CreateParticles.ParticlesVecRandom  = Vector2T<float>(-0.22f, 0.22f);

	DemoParticlePActor1->FxParticlesGroupCreate(CreateParticles);
}

void PsaGameV1Demo::GameCreateParticlesPActorCAMM(float number) {
	PsagManager::Fx::ParticleGenDESC CreateParticles;

	CreateParticles.PariclesNumber      = number;
	CreateParticles.ParticlesColorMode  = PsagManager::Fx::GenMode::ChannelsRG;
	CreateParticles.ParticlesLaunchMode = PsagManager::Fx::GenMode::PrtcPoly;

	CreateParticles.ParticlesCrRandom = Vector2T<float>(0.42f, 1.68f);
	CreateParticles.ParticlesCgRandom = Vector2T<float>(0.24f, 0.72f);

	CreateParticles.ParticlesLifeRandom = Vector2T<float>(70.0f, 100.0f);
	CreateParticles.ParticlesSizeRandom = Vector2T<float>(1.5f, 4.0f);
	CreateParticles.ParticlesPosRandom  = Vector2T<float>(-120.0f, 120.0f);
	CreateParticles.ParticlesVecRandom  = Vector2T<float>(0.58f, 0.72f);

	DemoParticlePActor2->FxParticlesGroupCreate(CreateParticles);
}

void PsaGameV1Demo::GameCreateParticlesBullet(float number, const Vector2T<float>& pos, int mode) {
	PsagManager::Fx::ParticleGenDESC CreateParticles;

	CreateParticles.PariclesNumber      = number;
	CreateParticles.ParticlesLaunchMode = PsagManager::Fx::GenMode::PrtcPoints;

	switch (mode) {
	case(0): { CreateParticles.ParticlesColorMode = PsagManager::Fx::GenMode::ChannelsGB; break; }
	case(1): { CreateParticles.ParticlesColorMode = PsagManager::Fx::GenMode::ChannelsRB; break; }
	}
	CreateParticles.ParticlesCrRandom = Vector2T<float>(0.25f, 1.75f);
	CreateParticles.ParticlesCgRandom = Vector2T<float>(0.25f, 1.75f);
	CreateParticles.ParticlesCbRandom = Vector2T<float>(0.25f, 1.75f);

	CreateParticles.ParticlesLifeRandom = Vector2T<float>(128.0f, 240.0f);
	CreateParticles.ParticlesSizeRandom = Vector2T<float>(0.32f, 1.38f);
	CreateParticles.ParticlesPosRandom  = Vector2T<float>(-1.0f, 1.0f);
	CreateParticles.ParticlesVecRandom  = Vector2T<float>(-1.2f, 1.2f);

	CreateParticles.ParticlesPosOffset = pos;

	DemoParticleBullet->FxParticlesGroupCreate(CreateParticles);
}

PawnActorFX::PawnActorFX(
    PsagActor::ActorsManager* Manager, Vector2T<PsagActor::ActorShader*> Shader, 
    const Vector2T<float>& Position
) {
    PsagActor::ActorDESC ConfigStarActor;

    // 卸载碰撞组件, 但是保留控制组件.
    ConfigStarActor.EnableCollision = false;

    ConfigStarActor.ActorPhysicsWorld   = "DemoPhysics";
    ConfigStarActor.ActorShaderResource = Shader.vector_x;

    ConfigStarActor.InitialRenderLayer = 52.0f;
    ConfigStarActor.InitialScale       = Vector2T<float>(1.0f, 1.0f);
    ConfigStarActor.InitialPosition    = Position;
    FxActorUnique[0] = Manager->CreateGameActor(PsagActorType::ActorTypeAllotter.ActorTypeIs("actor_fx"), ConfigStarActor);

    ConfigStarActor.ActorShaderResource = Shader.vector_y;
    ConfigStarActor.InitialRenderLayer  = 54.0f;
    FxActorUnique[1] = Manager->CreateGameActor(PsagActorType::ActorTypeAllotter.ActorTypeIs("actor_fx"), ConfigStarActor);

    ActorShader1 = Shader.vector_x;
    ActorShader2 = Shader.vector_y;
    ActorManager = Manager;

    PsagLow::PsagSupGraphicsOper::PsagGraphicsImageRawDat DecodeRawImage;
    PsagLow::PsagSupFilesysLoaderBin ParticleTexture("Test/ParticleImgTest.png");

    ActorParticles = new GraphicsEngineParticle::PsagGLEngineParticle(
        Vector2T<uint32_t>(),
        DecodeRawImage.DecodeImageRawData(ParticleTexture.GetDataBinary())
    );
    ActorParticles->SetParticleTwisted(2.0f);
}

void PawnActorFX::PawnActorFxRender() {
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
    // uniform set_brightness.
    ActorShader1->UniformSetContext([&]() { ActorShader1->UniformFP32("Brightness", FxActorAnimTimer * 0.5f); });

    FxActorObj1->ActorModifyScale(Vector2T<float>(FxActorAnimTimer, FxActorAnimTimer));
    FxActorObj2->ActorModifyScale(Vector2T<float>(FxActorAnimTimer * 1.7f, FxActorAnimTimer * 1.7f));

    ActorParticles->UpdateParticleData();
    ActorParticles->RenderParticleFX();

    FxActorAnimTimer += 0.002f;
    FxActorTimer += 0.02f;
    if (FxActorAnimTimer > 20.0f) FxActorAnimTimer = 20.0f;
}

PawnActorFX::~PawnActorFX() {
    ActorManager->DeleteGameActor(FxActorUnique[0]);
    ActorManager->DeleteGameActor(FxActorUnique[1]);
}