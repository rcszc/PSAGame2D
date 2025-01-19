// psag_manager_module_fx.
#include "psag_manager_module.hpp"

using namespace std;
using namespace PSAG_LOGGER;

namespace GameManagerCore {
	namespace GameFX {

		GameFxParticle::GameFxParticle(const GameFxParticleDESC& INIT_DESC) {
			// create new particle_system.
			FxParticleObject = new GraphicsEngineParticle::PsagGLEngineParticle(
				INIT_DESC.ParticleRenderResolution, 
				INIT_DESC.ParticleRenderTexture
			);
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

		bool GameFxParticle::FxParticlesGroupCreate(GraphicsEngineParticle::ParticleGeneratorBase* Generator) {
			if (Generator == nullptr) {
				PushLogger(LogError, PSAGM_MANAGER_FX_LABEL, "manager fx_particle generator is nullptr.");
				return false;
			}
			// particle generator => particle system.
			FxParticleObject->ParticleCreate(Generator);
			delete Generator; Generator = nullptr;
			return true;
		}

		bool GameFxParticle::FxParticlesAdd(const ParticleAttributes& ADD_PARTICLE) {
			FxParticleObject->GetParticleDataset()->push_back(ADD_PARTICLE);
			return ADD_PARTICLE.ParticleScaleSize > 0.0f;
		}

		bool GameFxParticle::FxParticlesAddDataset(const ParticlesDataset& ADD_PARTICLES) {
			if (ADD_PARTICLES.empty())
				return false;
			FxParticleObject->GetParticleDataset()->insert(
				FxParticleObject->GetParticleDataset()->end(), ADD_PARTICLES.begin(), ADD_PARTICLES.end()
			);
			return true;
		}

		void GameFxParticle::SetFxParticlesCenter(const Vector2T<float>& coord) {
			// particles calc center_coord.
			FxParticleObject->ParticlesCoordCenter = coord;
		}

		void GameFxParticle::SetFxParticlesRandRotate(bool rot_switch) {
			// false: * speed(0.0f), true: speed = basic(1.0f)
			FxParticleObject->SetParticleRotateSpeed((float)rot_switch);
		}

		void GameFxParticle::FxParticleRendering() {
			// particle system calc(update) => rendering.
			FxParticleObject->UpdateParticleData();
			FxParticleObject->RenderParticleFX();
		}

		GameFxSpriteSheet::GameFxSpriteSheet(const GameFxSpriteSheetDESC& INIT_DESC) {
			GraphicsEnginePVFX::SequencePlayer PlayParamsTemp = {};

			PlayParamsTemp.UaxisFrameNumber = INIT_DESC.SpriteSheetNumber.vector_x;
			PlayParamsTemp.VaxisFrameNumber = INIT_DESC.SpriteSheetNumber.vector_y;
			PlayParamsTemp.PlayerSpeedScale = INIT_DESC.SpriteSheetPlaySpeed;

			// texture shader blend_color(init).
			BackgroundColorBlend = INIT_DESC.SpriteSheetInitColorBg;

			FxSpriteSheetObject = new 
				GraphicsEnginePVFX::PsagGLEngineFxSequence(INIT_DESC.RenderTexture, PlayParamsTemp);
			PushLogger(LogInfo, PSAGM_MANAGER_FX_LABEL, "manager fx_sprite_sheet init.");
		}

		GameFxSpriteSheet::~GameFxSpriteSheet() {
			delete FxSpriteSheetObject;

			PushLogger(LogInfo, PSAGM_MANAGER_FX_LABEL, "manager fx_sprite_sheet free.");
		}

		void GameFxSpriteSheet::FxSpriteSheetRendering() {
			FxSpriteSheetObject->DrawFxSequence(BackgroundColorBlend);
		}

		VirTextureUnqiue GameFxSpriteSheet::FxSpriteSheetTexture() {
			return FxSpriteSheetObject->GetFxSequenceTexture();
		}
	}
}