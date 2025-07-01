// zero_project_bullet_fx.
#include "zero_project_bullet.h"

using namespace std;
using namespace PSAG_LOGGER;

void BulletParticle::InitParticleSystem(const Vector2T<uint32_t>& window_size) {
	PsagManager::SyncLoader::SyncEncDecImage DecodeRawImage;
	PsagManager::Fx::ParticleDESC CreateBulletParticle = {};

	auto BulletImage = PsagManager::SyncLoader::FSLD::EasyFileReadRawData(SYSPATH_REF("zpgame_particle_a.png"));
	
	CreateBulletParticle.ParticleRenderSize    = window_size;
	CreateBulletParticle.ParticlesDisturbance  = 4.0f;
	CreateBulletParticle.ParticleRenderTexture = DecodeRawImage.DecodeImageRawData(BulletImage);

	ParticleSystem.CreatePointer(CreateBulletParticle);
}

void BulletParticle::CreateParticleGuide(const Vector2T<float>& position) {
	PsagManager::Fx::ParticleGenShape* GCircle = new PsagManager::Fx::ParticleGenShape();

	GCircle->ConfigCreateNumber(148);
	GCircle->SettingShape(1);

	GCircle->ConfigGenPos    (position);
	GCircle->ConfigGenPosRand(Vector2T<float>(0.0f, 0.0f));
	GCircle->ConfigGenVector (Vector2T<float>(-3.2f, 3.2f));

	GCircle->ConfigLifeDispersion(Vector2T<float>(20.0f, 36.0f));
	GCircle->ConfigSizeDispersion(Vector2T<float>(1.92f, 2.58f));

	GCircle->ConfigRandomColorSystem(
		Vector2T<float>(0.16f, 0.58f), Vector2T<float>(0.2f, 0.2f), Vector2T<float>(0.42f, 1.58f),
		PsagManager::Fx::ParticleColorMode::ChannelsRGB
	);
	ParticleSystem.Get()->FxParticlesGroupCreate(GCircle);
}

void BulletParticle::CreateParticleFly(const Vector2T<float>& position, float number) {
	PsagManager::Fx::ParticleGenPoints* PCircle = new PsagManager::Fx::ParticleGenPoints();

	PCircle->ConfigCreateNumber(number);

	PCircle->ConfigGenPos    (position);
	PCircle->ConfigGenPosRand(Vector2T<float>(-0.12f, 0.12f));
	PCircle->ConfigGenVector (Vector2T<float>(-0.08f, 0.08f));

	PCircle->ConfigLifeDispersion(Vector2T<float>(72.0f, 160.0f));
	PCircle->ConfigSizeDispersion(Vector2T<float>(0.25f, 0.75f));

	PCircle->ConfigRandomColorSystem(
		Vector2T<float>(0.0f, 0.0f), Vector2T<float>(0.32f, 1.58f), Vector2T<float>(0.32f, 1.58f),
		PsagManager::Fx::ParticleColorMode::ChannelsGB
	);
	ParticleSystem.Get()->FxParticlesGroupCreate(PCircle);
}

void BulletParticle::CreateParticleFree0(const Vector2T<float>& position, float number) {
	PsagManager::Fx::ParticleGenShape* PCircle = new PsagManager::Fx::ParticleGenShape();

	PCircle->ConfigCreateNumber(number);
	PCircle->SettingShape(1);

	PCircle->ConfigGenPos    (position);
	PCircle->ConfigGenPosRand(Vector2T<float>(-3.25f, 3.25f));
	PCircle->ConfigGenVector (Vector2T<float>(-0.95f, 0.95f));

	PCircle->ConfigLifeDispersion(Vector2T<float>(20.0f, 40.0f));
	PCircle->ConfigSizeDispersion(Vector2T<float>(0.58f, 1.32f));

	PCircle->ConfigRandomColorSystem(
		Vector2T<float>(0.58f, 1.6f), Vector2T<float>(0.12f, 0.72f), Vector2T<float>(0.0f, 0.0f),
		PsagManager::Fx::ParticleColorMode::ChannelsRG
	);
	ParticleSystem.Get()->FxParticlesGroupCreate(PCircle);

	PsagManager::Fx::ParticleGenPoints* PPoints = new PsagManager::Fx::ParticleGenPoints();

	PPoints->ConfigCreateNumber(42);

	PPoints->ConfigGenPos    (position);
	PPoints->ConfigGenPosRand(Vector2T<float>(-5.28f, 5.28f));
	PPoints->ConfigGenVector (Vector2T<float>(-2.32f, 2.32f));

	PPoints->ConfigLifeDispersion(Vector2T<float>(100.0f, 200.0f));
	PPoints->ConfigSizeDispersion(Vector2T<float>(0.76f, 1.78f));

	PPoints->ConfigRandomColorSystem(
		Vector2T<float>(0.36f, 1.8f), Vector2T<float>(0.1f, 0.4f), Vector2T<float>(0.0f, 0.0f),
		PsagManager::Fx::ParticleColorMode::ChannelsRG
	);
	ParticleSystem.Get()->FxParticlesGroupCreate(PPoints);
}

void BulletParticle::CreateParticleFree1(const Vector2T<float>& position) {
	PsagManager::Fx::ParticleGenPoints* PPoints = new PsagManager::Fx::ParticleGenPoints();

	PPoints->ConfigCreateNumber(36);

	PPoints->ConfigGenPos(position);
	PPoints->ConfigGenPosRand(Vector2T<float>(-5.28f, 5.28f));
	PPoints->ConfigGenVector (Vector2T<float>(-2.32f, 2.32f));

	PPoints->ConfigLifeDispersion(Vector2T<float>(72.0f, 160.0f));
	PPoints->ConfigSizeDispersion(Vector2T<float>(0.58f, 1.28f));

	PPoints->ConfigRandomColorSystem(
		Vector2T<float>(0.32f, 1.6f), Vector2T<float>(0.0f, 0.0f), Vector2T<float>(0.0f, 0.0f),
		PsagManager::Fx::ParticleColorMode::ChannelsRB
	);
	ParticleSystem.Get()->FxParticlesGroupCreate(PPoints);
}

void BulletParticle::DebugParticleSystem() {
	ImGui::Begin("ParticleSystem Debug");
	{
		auto Params = ParticleSystem.Get()->GetFxParticlesState();
		ImGui::Text("Particles Number: %u, Bytes: %u", Params.DarwParticlesNumber, Params.DarwDatasetBytes);
		ImGui::Text("Particles Back FPS: %.3f", Params.BPT_RunFramerate);
		ImGui::Text("Particles Back MEM: %.3f mib", Params.BPT_BackMemoryUsed);
	}
	ImGui::End();
}

BulletParticle::~BulletParticle() {
	ParticleSystem.DeletePointer();
}