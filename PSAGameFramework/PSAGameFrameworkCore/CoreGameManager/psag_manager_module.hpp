// psag_manager_module. RCSZ. [middle_level_module]

#ifndef __PSAG_MANAGER_MODULE_HPP
#define __PSAG_MANAGER_MODULE_HPP
// graphics & collect engine => manager_module.
#include "../CoreFramework/GraphicsEngine/psag_graphics_engine.h"
#include "../CoreFramework/CollectEngine/psag_collect_engine.h"

namespace GameManagerCore {

	namespace GameFX {
		StaticStrLABEL PSAGM_MANAGER_FX_LABEL = "PSAG_MANAGER_FX";

		struct GameFxCreateParticleDESC {
			float PariclesNumber;

			Vector2T<float> ParticlesLifeRandom;
			Vector2T<float> ParticlesSizeRandom;

			Vector2T<float> ParticlesCrRandom; // color_red.
			Vector2T<float> ParticlesCgRandom; // color_green.
			Vector2T<float> ParticlesCbRandom; // color_blue.
			// color: rgb draw_mode.
			GraphicsEngineParticle::ColorChannelMode ParticlesColorMode;

			Vector2T<float> ParticlesVecRandom;
			Vector2T<float> ParticlesPosRandom;
			Vector3T<float> ParticlesPosOffsetRandom;

			GraphicsEngineParticle::EmittersMode ParticlesLaunchMode;

			GameFxCreateParticleDESC() :
				ParticlesColorMode (GraphicsEngineParticle::ChannelsRGB),
				ParticlesLaunchMode(GraphicsEngineParticle::PrtcPoints),
				PariclesNumber     (10.0f),

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

			GameFxParticleDESC() :
				ParticleRenderResolution(Vector2T<uint32_t>(1080.0f, 1920.0f)),
				ParticleRenderTexture   ({}),
				ParticleUpdateCalcMode  (UpdateDefault)
			{}
		};

		class GameFxParticle {
		protected:
			GraphicsEngineParticle::PsagGLEngineParticle* FxParticleObject = nullptr;
		public:
			GameFxParticle(const GameFxParticleDESC& INIT_DESC);
			~GameFxParticle();

			bool FxParticleCreateGroup(const GameFxCreateParticleDESC& CREATE_DESC);

			void FxParticleRendering();
		};
	}
}

#endif