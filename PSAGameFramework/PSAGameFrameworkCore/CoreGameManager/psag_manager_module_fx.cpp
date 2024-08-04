// psag_manager_module_fx.
#include "psag_manager_module.hpp"

using namespace std;
using namespace PSAG_LOGGER;

namespace GameManagerCore {
	namespace GameFX {

		GameFxParticle::GameFxParticle(const GameFxParticleDESC& INIT_DESC) {
			FxParticleObject = new 
				GraphicsEngineParticle::PsagGLEngineParticle(INIT_DESC.ParticleRenderResolution, INIT_DESC.ParticleRenderTexture);
			// set particle calc mode.
			FxParticleObject->ParticleClacMode(
				(GraphicsEngineParticle::ParticleCalcMode)INIT_DESC.ParticleUpdateCalcMode
			);
			FxParticleObject->SetParticleTwisted(INIT_DESC.ParticlesDisturbance);
			PushLogger(LogInfo, PSAGM_MANAGER_FX_LABEL, "manager fx_particle init.");
		}

		GameFxParticle::~GameFxParticle() {
			delete FxParticleObject;

			PushLogger(LogInfo, PSAGM_MANAGER_FX_LABEL, "manager fx_particle free.");
		}

		bool GameFxParticle::FxParticlesGroupCreate(const GameFxCreateParticleDESC& CREATE_DESC) {
			GraphicsEngineParticle::ParticleGenerator ParticleGroupCreate = {};

			if (!ParticleGroupCreate.ConfigCreateNumber(CREATE_DESC.PariclesNumber))
				return false;
			ParticleGroupCreate.ConfigCreateMode(CREATE_DESC.ParticlesLaunchMode);

			ParticleGroupCreate.ConfigLifeDispersion(CREATE_DESC.ParticlesLifeRandom);
			ParticleGroupCreate.ConfigSizeDispersion(CREATE_DESC.ParticlesSizeRandom);

			ParticleGroupCreate.ConfigRandomColorSystem(
				CREATE_DESC.ParticlesCrRandom,
				CREATE_DESC.ParticlesCgRandom,
				CREATE_DESC.ParticlesCbRandom
			);
			ParticleGroupCreate.ConfigRandomDispersion(
				CREATE_DESC.ParticlesVecRandom,
				CREATE_DESC.ParticlesPosRandom,
				CREATE_DESC.ParticlesPosOffsetRandom
			);
			// particle generator => particle system.
			FxParticleObject->ParticleCreate(&ParticleGroupCreate);
			return true;
		}

		bool GameFxParticle::FxParticlesAdd(const ParticleAttributes& ADD_PARTICLE) {
			FxParticleObject->GetParticleDataset()->push_back(ADD_PARTICLE);
			return true; // non-err 20240729.
		}

		bool GameFxParticle::FxParticlesAddDataset(const ParticlesDataset& ADD_PARTICLES) {
			if (ADD_PARTICLES.empty())
				return false;
			FxParticleObject->GetParticleDataset()->insert(
				FxParticleObject->GetParticleDataset()->end(), ADD_PARTICLES.begin(), ADD_PARTICLES.end()
			);
			return true;
		}

		void GameFxParticle::FxParticleRendering() {
			// particle system calc(update) => rendering.
			FxParticleObject->UpdateParticleData();
			FxParticleObject->RenderParticleFX();
		}

		GameFxSpriteSheet::GameFxSpriteSheet(const GameFxSpriteSheetDESC& INIT_DESC) {

		}

		GameFxSpriteSheet::~GameFxSpriteSheet() {

		}
	}
}