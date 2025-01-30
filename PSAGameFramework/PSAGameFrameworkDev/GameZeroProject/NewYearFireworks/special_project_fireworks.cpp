// special_project_fireworks.
#include "special_project.h"

using namespace std;
using namespace PSAG_LOGGER;

void FireworksParticle::InitParticleSystem(const Vector2T<uint32_t>& window_size) {
	PsagManager::SyncLoader::SyncEncDecImage DecodeRawImage;
	PsagManager::Fx::ParticleDESC CreateParticle = {};

	auto ParticleImage = LOADFILE::EasyFileReadRawData(SPE_SYSPATH_REF("spe_particle.png"));

	CreateParticle.ParticleRenderResolution = window_size;
	CreateParticle.ParticlesDisturbance     = 4.0f;
	CreateParticle.ParticleRenderTexture    = DecodeRawImage.DecodeImageRawData(ParticleImage);

	ParticleSystem.CreatePointer(CreateParticle);
}

void FireworksParticle::CreateParticleFly(const Vector2T<float>& position, float number) {
	PsagManager::Fx::ParticleGenPoints* CreateParticleOBJ = new PsagManager::Fx::ParticleGenPoints();
	
	CreateParticleOBJ->ConfigCreateNumber(number);

	CreateParticleOBJ->ConfigGenPos    (position);
	CreateParticleOBJ->ConfigGenPosRand(Vector2T<float>(-0.28f, 0.28f));
	CreateParticleOBJ->ConfigGenVector (Vector2T<float>(-0.18f, 0.18f));

	CreateParticleOBJ->ConfigLifeDispersion(Vector2T<float>(128.0f, 256.0f));
	CreateParticleOBJ->ConfigSizeDispersion(Vector2T<float>(0.25f, 0.75f));

	CreateParticleOBJ->ConfigRandomColorSystem(
		Vector2T<float>(0.58f, 1.8f), Vector2T<float>(0.58f, 1.8f), Vector2T<float>(0.58f, 1.8f),
		PsagManager::Fx::ParticleColorMode::ChannelsGB
	);
	ParticleSystem.Get()->FxParticlesGroupCreate(CreateParticleOBJ);
}

void FireworksParticle::CreateParticleFreeA(const Vector2T<float>& position, float number) {
	PsagManager::Fx::ParticleGenPoints* CreateFree1 = new PsagManager::Fx::ParticleGenPoints();

	CreateFree1->ConfigCreateNumber(number - 42.0f);

	CreateFree1->ConfigGenPos    (position);
	CreateFree1->ConfigGenPosRand(Vector2T<float>(-5.28f, 5.28f));
	CreateFree1->ConfigGenVector (Vector2T<float>(-2.32f, 2.32f));

	CreateFree1->ConfigLifeDispersion(Vector2T<float>(200.0f, 500.0f));
	CreateFree1->ConfigSizeDispersion(Vector2T<float>(0.75f, 1.75f));

	CreateFree1->ConfigRandomColorSystem(
		Vector2T<float>(0.32f, 1.72f), Vector2T<float>(0.0f, 0.0f), Vector2T<float>(0.0f, 0.0f),
		PsagManager::Fx::ParticleColorMode::ChannelsRB
	);
	ParticleSystem.Get()->FxParticlesGroupCreate(CreateFree1);
}

void FireworksParticle::CreateParticleFreeB(const Vector2T<float>& position, float number) {
	PsagManager::Fx::ParticleGenShape* CreateFree0 = new PsagManager::Fx::ParticleGenShape();

	CreateFree0->ConfigCreateNumber(number);
	CreateFree0->SettingShape(1);

	CreateFree0->ConfigGenPos    (position);
	CreateFree0->ConfigGenPosRand(Vector2T<float>(-3.25f, 3.25f));
	CreateFree0->ConfigGenVector (Vector2T<float>(-0.95f, 0.95f));

	CreateFree0->ConfigLifeDispersion(Vector2T<float>(100.0f, 220.0f));
	CreateFree0->ConfigSizeDispersion(Vector2T<float>(0.75f, 1.95f));

	CreateFree0->ConfigRandomColorSystem(
		Vector2T<float>(0.72f, 1.92f), Vector2T<float>(0.16f, 0.58f), Vector2T<float>(1.0f, 1.0f),
		PsagManager::Fx::ParticleColorMode::ChannelsRG
	);
	ParticleSystem.Get()->FxParticlesGroupCreate(CreateFree0);

	PsagManager::Fx::ParticleGenPoints* CreateFree1 = new PsagManager::Fx::ParticleGenPoints();

	CreateFree1->ConfigCreateNumber(number - 42.0f);

	CreateFree1->ConfigGenPos    (position);
	CreateFree1->ConfigGenPosRand(Vector2T<float>(-5.28f, 5.28f));
	CreateFree1->ConfigGenVector (Vector2T<float>(-2.32f, 2.32f));

	CreateFree1->ConfigLifeDispersion(Vector2T<float>(200.0f, 500.0f));
	CreateFree1->ConfigSizeDispersion(Vector2T<float>(0.32f, 1.28f));

	CreateFree1->ConfigRandomColorSystem(
		Vector2T<float>(0.58f, 0.72f), Vector2T<float>(1.0f, 1.0f), Vector2T<float>(0.78f, 1.92f),
		PsagManager::Fx::ParticleColorMode::ChannelsRB
	);
	ParticleSystem.Get()->FxParticlesGroupCreate(CreateFree1);
}

FireworksParticle::~FireworksParticle() {
	ParticleSystem.DeletePointer();
}