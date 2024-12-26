// framework_dev_manager. RCSZ. 20240727.
// dev_support: manager

#ifndef __FRAMEWORK_DEV_MANAGER_H
#define __FRAMEWORK_DEV_MANAGER_H
#include "../PSAGameFrameworkCore/CoreFramework/psag_mainevent_support.h"

namespace PsagManager {
	namespace Fx {
		namespace GenMode = GameManagerCore::GameFX::ParticleMode;

		using ParticleDESC    = GameManagerCore::GameFX::GameFxParticleDESC;
		using ParticleGenDESC = GameManagerCore::GameFX::GameFxCreateParticleDESC;
		using Particle        = GameManagerCore::GameFX::GameFxParticle;

		using ParticleCalcMode = GameManagerCore::GameFX::ParticleUpdateMode;

		using ParticleSrcAttri = GameManagerCore::GameFX::ParticleAttributes;
		using ParticleSrcArray = GameManagerCore::GameFX::ParticlesDataset;

		using SpriteSheetDESC = GameManagerCore::GameFX::GameFxSpriteSheetDESC;
		using SpriteSheet     = GameManagerCore::GameFX::GameFxSpriteSheet;
	}

	namespace SceneFinal = GameManagerCore::GameSceneForFinal;
	using FINAL_PARAMS = GameManagerCore::GameSceneForFinal::SceneFinalParams;

	namespace FxView     = GameManagerCore::GameView;
	namespace Maths      = GameManagerCore::GameMathsTools;
	namespace SyncLoader = GameManagerCore::GameSyncLoader;

	namespace Tools {
		namespace RAND   = ToolkitsEngineRandom::Func;
		namespace Random = ToolkitsEngineRandom;
		namespace Pawn   = ToolkitsEnginePawn;
		namespace Camera = ToolkitsEngineCamera;
		namespace Timer  = ToolkitsEngineTimerClock;
		namespace Sound  = ToolkitsEngineSound;
	}
	// imgui gui extension_contral.
	namespace GuiTools = ToolkitsEngineGuiExt;
	// platform(windows) api tools.
	namespace Win32Tools = PsagLow::WindowsTools;

#define AudioConvert PsagSupAudioRawCVT
}

#endif