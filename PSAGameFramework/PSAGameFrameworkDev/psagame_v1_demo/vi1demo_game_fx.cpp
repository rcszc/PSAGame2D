// vi1demo_game_fx.
#include "v1demo_game.h"

using namespace std;
using namespace PSAG_LOGGER;

void PsaGameV1Demo::GameInitParticleSystem(const Vector2T<uint32_t>& w_size) {
	PsagManager::SyncLoader::SyncDecodeImage DecodeRawImage;
	PsagManager::Fx::ParticleDESC CreatePActorParticle = {};

	PsagManager::SyncLoader::SyncBinFileLoad PActorImage("demo_v1_material/psag_v1_particle2.png");

	CreatePActorParticle.ParticleRenderResolution = w_size;
	CreatePActorParticle.ParticlesDisturbance     = 0.0f;
	CreatePActorParticle.ParticleRenderTexture    = DecodeRawImage.DecodeImageRawData(PActorImage.GetDataBinary());

	DemoParticlePActor = new PsagManager::Fx::Particle(CreatePActorParticle);
}

void PsaGameV1Demo::GameCreateParticlesPActor(float number) {
	PsagManager::Fx::ParticleGenDESC CreateParticles;

	CreateParticles.PariclesNumber = number;
	CreateParticles.ParticlesColorMode  = PsagManager::Fx::GenMode::ChannelsRGB;
	CreateParticles.ParticlesLaunchMode = PsagManager::Fx::GenMode::PrtcPoints;

	CreateParticles.ParticlesCgRandom   = Vector2T<float>(0.32f, 1.0f);
	CreateParticles.ParticlesLifeRandom = Vector2T<float>(150.0f, 250.0f);
	CreateParticles.ParticlesSizeRandom = Vector2T<float>(1.8f, 4.8f);
	CreateParticles.ParticlesPosRandom  = Vector2T<float>(-38.0f, 38.0f);
	CreateParticles.ParticlesVecRandom  = Vector2T<float>(-0.22f, 0.22f);

	DemoParticlePActor->FxParticlesGroupCreate(CreateParticles);
}