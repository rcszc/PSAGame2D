// vi1demo_game_fx.
#include "v1demo_game.h"

using namespace std;
using namespace PSAG_LOGGER;

void PsaGameV1Demo::GameInitParticleSystem(const Vector2T<uint32_t>& w_size) {
	PsagManager::SyncLoader::SyncEncDecImage DecodeRawImage;
	PsagManager::Fx::ParticleDESC CreatePActorParticle = {};

	auto PActorImage1 = PsagManager::SyncLoader::FSLD::EasyFileReadRawData("demo_v1_material/psag_v1_particle2.png");
	auto PActorImage2 = PsagManager::SyncLoader::FSLD::EasyFileReadRawData("demo_v1_material/psag_v1_particle1.png");
	auto PActorImage3 = PsagManager::SyncLoader::FSLD::EasyFileReadRawData("demo_v1_material/psag_v1_particle0.png");

	CreatePActorParticle.ParticleRenderResolution = w_size;
	CreatePActorParticle.ParticlesDisturbance     = 0.0f;
	CreatePActorParticle.ParticleRenderTexture    = DecodeRawImage.DecodeImageRawData(PActorImage1);

	DemoParticlePActor1 = new PsagManager::Fx::Particle(CreatePActorParticle);

	// ∏¥”√ particle DESC.
	CreatePActorParticle.ParticleRenderTexture = DecodeRawImage.DecodeImageRawData(PActorImage2);
	DemoParticlePActor2 = new PsagManager::Fx::Particle(CreatePActorParticle);

	// ∏¥”√ particle DESC.
	CreatePActorParticle.ParticleRenderTexture = DecodeRawImage.DecodeImageRawData(PActorImage3);
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