// zero_project_bullet_fx.
#include "zero_project_bullet.h"

using namespace std;
using namespace PSAG_LOGGER;

void BulletParticle::InitParticleSystem(const Vector2T<uint32_t>& window_size) {
	PsagManager::SyncLoader::SyncEncDecImage DecodeRawImage;
	PsagManager::Fx::ParticleDESC CreateBulletParticle = {};

	auto BulletImage = PsagManager::SyncLoader::FSLD::EasyFileReadRawData(SYSPATH_REF("zpgame_particle_a.png"));
	
	CreateBulletParticle.ParticleRenderResolution = window_size;
	CreateBulletParticle.ParticlesDisturbance     = 4.0f;
	CreateBulletParticle.ParticleRenderTexture    = DecodeRawImage.DecodeImageRawData(BulletImage);

	ParticleSystem.CreatePointer(CreateBulletParticle);
}

void BulletParticle::CreateParticleFly(const Vector2T<float>& position, float number) {
	PsagManager::Fx::ParticleGenPoints* CreateParticleOBJ = new PsagManager::Fx::ParticleGenPoints();

	CreateParticleOBJ->ConfigCreateNumber(number);

	CreateParticleOBJ->ConfigGenPos    (position);
	CreateParticleOBJ->ConfigGenPosRand(Vector2T<float>(-0.28f, 0.28f));
	CreateParticleOBJ->ConfigGenVector (Vector2T<float>(-0.18f, 0.18f));

	CreateParticleOBJ->ConfigLifeDispersion(Vector2T<float>(128.0f, 256.0f));
	CreateParticleOBJ->ConfigSizeDispersion(Vector2T<float>(0.25f, 0.75f));

	CreateParticleOBJ->ConfigRandomColorSystem(
		Vector2T<float>(0.72f, 1.92f), Vector2T<float>(0.16f, 0.58f), Vector2T<float>(1.0f, 1.0f),
		PsagManager::Fx::ParticleColorMode::ChannelsRG
	);
	ParticleSystem.Get()->FxParticlesGroupCreate(CreateParticleOBJ);
}

void BulletParticle::CreateParticleFreeB(const Vector2T<float>& position, float number) {
	PsagManager::Fx::ParticleGenShape* CreateFree0 = new PsagManager::Fx::ParticleGenShape();

	CreateFree0->ConfigCreateNumber(number);
	CreateFree0->SettingShape(1);

	CreateFree0->ConfigGenPos    (position);
	CreateFree0->ConfigGenPosRand(Vector2T<float>(-3.25f, 3.25f));
	CreateFree0->ConfigGenVector (Vector2T<float>(-0.95f, 0.95f));

	CreateFree0->ConfigLifeDispersion(Vector2T<float>(100.0f, 220.0f));
	CreateFree0->ConfigSizeDispersion(Vector2T<float>(0.75f, 1.95f));

	CreateFree0->ConfigRandomColorSystem(
		Vector2T<float>(0.58f, 1.8f), Vector2T<float>(0.58f, 1.8f), Vector2T<float>(0.58f, 1.8f),
		PsagManager::Fx::ParticleColorMode::ChannelsGB
	);
	ParticleSystem.Get()->FxParticlesGroupCreate(CreateFree0);

	PsagManager::Fx::ParticleGenPoints* CreateFree1 = new PsagManager::Fx::ParticleGenPoints();

	CreateFree1->ConfigCreateNumber(32);

	CreateFree1->ConfigGenPos    (position);
	CreateFree1->ConfigGenPosRand(Vector2T<float>(-5.28f, 5.28f));
	CreateFree1->ConfigGenVector (Vector2T<float>(-2.32f, 2.32f));

	CreateFree1->ConfigLifeDispersion(Vector2T<float>(200.0f, 500.0f));
	CreateFree1->ConfigSizeDispersion(Vector2T<float>(0.75f, 1.75f));

	CreateFree1->ConfigRandomColorSystem(
		Vector2T<float>(0.32f, 0.85f), Vector2T<float>(1.0f, 1.0f), Vector2T<float>(0.85f, 1.85f),
		PsagManager::Fx::ParticleColorMode::ChannelsRB
	);
	ParticleSystem.Get()->FxParticlesGroupCreate(CreateFree1);
}

BulletParticle::~BulletParticle() {
	ParticleSystem.DeletePointer();
}