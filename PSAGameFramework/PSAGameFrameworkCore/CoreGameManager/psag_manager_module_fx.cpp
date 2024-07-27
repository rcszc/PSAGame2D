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
			PushLogger(LogInfo, PSAGM_MANAGER_FX_LABEL, "manager fx_particle init.");
		}

		GameFxParticle::~GameFxParticle() {
			delete FxParticleObject;

			PushLogger(LogInfo, PSAGM_MANAGER_FX_LABEL, "manager fx_particle init.");
		}

		bool GameFxParticle::FxParticleCreateGroup(const GameFxCreateParticleDESC& CREATE_DESC) {
			GraphicsEngineParticle::ParticleGenerator ParticleGroupCreate = {};

			ParticleGroupCreate.ConfigCreateNumber(CREATE_DESC.PariclesNumber);
			ParticleGroupCreate.ConfigCreateMode  (CREATE_DESC.ParticlesLaunchMode);

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
		}

		void GameFxParticle::FxParticleRendering() {
			// particle system calc(update) => rendering.
			FxParticleObject->UpdateParticleData();
			FxParticleObject->RenderParticleFX();
		}
	}
}