// psag_manager_module. RCSZ. [middle_level_module]

#ifndef __PSAG_MANAGER_MODULE_HPP
#define __PSAG_MANAGER_MODULE_HPP
// graphics & collect engine => manager_module.
#include "../CoreFramework/GraphicsEngine/psag_graphics_engine.h"
#include "../CoreFramework/CollectEngine/psag_collect_engine.h"

namespace GameManagerCore {

	namespace GameFX {
		StaticStrLABEL PSAGM_MANAGER_FX_LABEL = "PSAG_MANAGER_FX";

		// ******************************** FX-Particle ********************************

		namespace PartcMode = GraphicsEngineParticle::ParticlesGenMode;

		struct GameFxCreateParticleDESC {
			float PariclesNumber;

			Vector2T<float> ParticlesLifeRandom;
			Vector2T<float> ParticlesSizeRandom;

			Vector2T<float> ParticlesCrRandom; // color_red.
			Vector2T<float> ParticlesCgRandom; // color_green.
			Vector2T<float> ParticlesCbRandom; // color_blue.
			// color: rgb draw_mode.
			PartcMode::ColorChannelMode ParticlesColorMode;

			Vector2T<float> ParticlesVecRandom;
			Vector2T<float> ParticlesPosRandom;
			Vector3T<float> ParticlesPosOffsetRandom;

			PartcMode::EmittersMode ParticlesLaunchMode;

			GameFxCreateParticleDESC() :
				ParticlesColorMode  (PartcMode::ChannelsRGB),
				ParticlesLaunchMode (PartcMode::PrtcPoints),
				PariclesNumber      (10.0f),

				ParticlesLifeRandom(Vector2T<float>(128.0f, 256.0f)),
				ParticlesSizeRandom(Vector2T<float>(1.0f, 2.0f)),

				ParticlesCrRandom (Vector2T<float>(0.0f, 1.0f)),
				ParticlesCgRandom (Vector2T<float>(0.0f, 1.0f)),
				ParticlesCbRandom (Vector2T<float>(0.0f, 1.0f)),

				ParticlesVecRandom      (Vector2T<float>(-1.0f, 1.0f)),
				ParticlesPosRandom      (Vector2T<float>(-1.0f, 1.0f)),
				ParticlesPosOffsetRandom(Vector3T<float>(0.0f, 0.0f, 0.0f))
			{}
		};

		// value = 'GraphicsEngineParticle::ParticleCalcMode'
		enum ParticleUpdateMode {
			UpdateDefault   = 1 << 1, // 默认计算模式.
			UpdateOpenMP    = 1 << 2, // 并行计算模式(OMP).
			UpdateEmptyOper = 1 << 3  // 空计算模式(手动接管).
		};

		struct GameFxParticleDESC {
			Vector2T<uint32_t> ParticleRenderResolution;
			ImageRawData       ParticleRenderTexture;

			ParticleUpdateMode ParticleUpdateCalcMode;
			// particles texture_coord disturbance.
			float ParticlesDisturbance;

			GameFxParticleDESC() :
				ParticleRenderResolution(Vector2T<uint32_t>(1080, 1920)),
				ParticleRenderTexture   ({}),
				ParticleUpdateCalcMode  (UpdateDefault),
				ParticlesDisturbance    (0.0f)
			{}
		};

		using ParticleAttributes = GraphicsEngineParticle::ParticleAttributes;
		using ParticlesDataset = std::vector<ParticleAttributes>;

		class GameFxParticle {
		protected:
			GraphicsEngineParticle::PsagGLEngineParticle* FxParticleObject = nullptr;
		public:
			GameFxParticle(const GameFxParticleDESC& INIT_DESC);
			~GameFxParticle();

			bool FxParticlesGroupCreate(const GameFxCreateParticleDESC& CREATE_DESC);

			bool FxParticlesAdd       (const ParticleAttributes& ADD_PARTICLE);
			bool FxParticlesAddDataset(const ParticlesDataset& ADD_PARTICLES);

			size_t GetFxParticlesNumber()    { return FxParticleObject->GetParticleState().DarwParticlesNumber; }
			size_t GetFxParticlesDataCount() { return FxParticleObject->GetParticleState().DarwDatasetSize; }

			ParticlesDataset* FxParticleSourceData() {
				// particle system src_dataset (attributes_struct) ptr.
				return FxParticleObject->GetParticleDataset();
			}

			void FxParticleRendering();
		};

		// ******************************** FX-SpriteSheet ********************************

		struct GameFxSpriteSheetDESC {
			float           SpriteSheetPlaySpeed;
			Vector2T<float> SpriteSheetNumber;

			ImageRawData RenderTexture;

			GameFxSpriteSheetDESC() :
				SpriteSheetPlaySpeed(1.0f),
				SpriteSheetNumber   (Vector2T<float>(1.0f, 1.0f)),

				RenderTexture({})
			{}
		};

		class GameFxSpriteSheet {
		protected:
			GraphicsEnginePVFX::PsagGLEngineFxSequence* FxSpriteSheetObject = nullptr;
		public:
			GameFxSpriteSheet(const GameFxSpriteSheetDESC& INIT_DESC);
			~GameFxSpriteSheet();
		};
	}
}

#endif