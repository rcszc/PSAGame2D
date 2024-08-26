// framework_dev_manager. RCSZ. 20240727.
// dev_support: manager

#ifndef __FRAMEWORK_DEV_MANAGER_H
#define __FRAMEWORK_DEV_MANAGER_H
#include "../PSAGameFrameworkCore/CoreFramework/psag_mainevent_support.h"

namespace PsagManager {
	namespace Fx {
		namespace GenMode = GameManagerCore::GameFX::PartcMode;

		using ParticleDESC    = GameManagerCore::GameFX::GameFxParticleDESC;
		using ParticleGenDESC = GameManagerCore::GameFX::GameFxCreateParticleDESC;
		using Particle        = GameManagerCore::GameFX::GameFxParticle;

		using ParticleCalcMode = GameManagerCore::GameFX::ParticleUpdateMode;

		using ParticleSrcAttri = GameManagerCore::GameFX::ParticleAttributes;
		using ParticleSrcArray = GameManagerCore::GameFX::ParticlesDataset;

		using SpriteSheetDESC = GameManagerCore::GameFX::GameFxSpriteSheetDESC;
		using SpriteSheet     = GameManagerCore::GameFX::GameFxSpriteSheet;
	}
	namespace FxView     = GameManagerCore::GameView;
	namespace Maths      = GameManagerCore::GameMathsTools;
	namespace SyncLoader = GameManagerCore::GameSyncLoader;

	namespace Tools {
		namespace RAND   = CollectEngineRandom::Func;
		namespace Random = CollectEngineRandom;
		namespace Pawn   = CollectEnginePawn;
		namespace Camera = CollectEngineCamera;
		namespace Timer  = CollectEngineTimerClock;
	}
}

#endif