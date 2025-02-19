// psag_manager_module. RCSZ. [middle_level_module]

#ifndef __PSAG_MANAGER_MODULE_HPP
#define __PSAG_MANAGER_MODULE_HPP
// graphics & collect engine => manager_module.
#define ENABLE_LOWMODULE_FILESYS
#include "../CoreFramework/GraphicsEngine/psag_graphics_engine.h"
#include "../CoreFramework/ToolkitsEngine/psag_toolkits_engine.h"

namespace GameManagerCore {
	namespace GameFX {
		namespace System = GraphicsEngineParticle;
		StaticStrLABEL PSAGM_MANAGER_FX_LABEL = "PSAG_MANAGER_FX";
		// ******************************** FX-Particle ********************************

		// particle computing mode type.
		enum ParticleUpdateMode {
			CALC_DEFAULT  = 1 << 1, // main thread.
			CALC_PARALLEL = 1 << 2, // multi thread.
			CALC_NO_CALC  = 1 << 3  // not calc.
		};
		using ParticleColorMode = GraphicsEngineParticle::ColorChannelMode;

		struct GameFxParticleDESC {
			Vector2T<uint32_t> ParticleRenderResolution;
			ImageRawData       ParticleRenderTexture;

			ParticleUpdateMode ParticleUpdateCalcMode;
			// particles texture_coord disturbance.
			float ParticlesDisturbance;

			GameFxParticleDESC() :
				ParticleRenderResolution(Vector2T<uint32_t>(1080, 1920)),
				ParticleRenderTexture   ({}),
				ParticleUpdateCalcMode  (CALC_DEFAULT),
				ParticlesDisturbance    (0.0f)
			{}
		};

		using ParticleAttributes = GraphicsEngineParticle::ParticleAttributes;
		using ParticlesDataset   = std::vector<ParticleAttributes>;

		class GameFxParticle {
		protected:
			GraphicsEngineParticle::PsagGLEngineParticle* FxParticleObject = nullptr;
		public:
			GameFxParticle(const GameFxParticleDESC& INIT_DESC);
			~GameFxParticle();

			// generator object destroyed by particle systems.
			bool FxParticlesGroupCreate(
				GraphicsEngineParticle::ParticleGeneratorBase* Generator
			);
			bool FxParticlesAdd       (const ParticleAttributes& ADD_PARTICLE);
			bool FxParticlesAddDataset(const ParticlesDataset& ADD_PARTICLES);

			size_t GetFxParticlesNumber()    { return FxParticleObject->GetParticleState().DarwParticlesNumber; }
			size_t GetFxParticlesDataCount() { return FxParticleObject->GetParticleState().DarwDatasetSize; }

			void SetFxParticlesCenter(const Vector2T<float>& coord);
			// particle random_angle rotate(speed), false => 0.0deg.
			void SetFxParticlesRandRotate(bool rot_switch);

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
			Vector4T<float> SpriteSheetInitColorBg;

			ImageRawData RenderTexture;

			GameFxSpriteSheetDESC() :
				SpriteSheetPlaySpeed  (1.0f),
				SpriteSheetNumber     (Vector2T<float>(1.0f, 1.0f)),
				SpriteSheetInitColorBg(Vector4T<float>(1.0f, 1.0f, 1.0f, 1.0f)),

				RenderTexture({})
			{}
		};

		class GameFxSpriteSheet {
		protected:
			GraphicsEnginePVFX::PsagGLEngineFxSequence* FxSpriteSheetObject = nullptr;
		public:
			GameFxSpriteSheet(const GameFxSpriteSheetDESC& INIT_DESC);
			~GameFxSpriteSheet();

			size_t GetFxSpriteSheetCount() {
				return FxSpriteSheetObject->PlayerCyclesCount;
			}
			Vector4T<float> BackgroundColorBlend = Vector4T<float>(1.0f, 1.0f, 1.0f, 1.0f);
			void FxSpriteSheetRendering();

			VirTextureUnique FxSpriteSheetTexture();
		};
	}

	namespace PointerManager {
		// version 0.1.0, 2025_01_03 RCSZ.
		// update: 2025.01.08.
		template<typename T>
		// not 'shared_ptr', non ref_counter.
		class DefinePointer {
		private:
			T* PointerDefine;
			std::string PointerInfo = {};
		public:
			DefinePointer() : PointerDefine(nullptr) {}
			~DefinePointer() {
				if (PointerDefine != nullptr) {
					delete PointerDefine; PointerDefine = nullptr;
				}
			}
			// create ptr, return address.
			template <typename... Args>
			size_t CreatePointer(Args&&... args) {
				PointerDefine = new T(std::forward<Args>(args)...);
				return (size_t)PointerDefine;
			}
			// delete ptr, return address.
			size_t DeletePointer() {
				if (PointerDefine == nullptr) return NULL;
				size_t PtrAddr = (size_t)PointerDefine;
				delete PointerDefine;
				PointerDefine = nullptr;
				return PtrAddr;
			}
			T* Get() { return PointerDefine; }
			// RTTI: object info string.
			std::string GetObjectINFO() {
				// rtti get object info.
				PointerInfo = typeid(T).name();
				return PointerInfo;
			};
		};
	}

	namespace GameSceneForFinal {
		StaticStrLABEL PSAGM_MANAGER_FINAL_LABEL = "PSAG_MANAGER_FINAL";

		class FinalParamsLerpSystem {
		protected:
			void ParamsLerpFloat1(float* value, float target);
			void ParamsLerpFloat2(Vector2T<float>* value, const Vector2T<float>& target);
			void ParamsLerpFloat3(Vector3T<float>* value, const Vector3T<float>& target);

			float LERP_TIMESETP_SEPPD = 1.0f;
		};

		using SceneFinalParams = GraphicsEngineFinal::FinalFxParameters;
		class GameFinalProcessing :public FinalParamsLerpSystem {
		protected:
			SceneFinalParams* FrameworkFinalParamsPtr = nullptr;
			SceneFinalParams CurrentFinalParams = {};

			std::vector<std::pair<std::string, SceneFinalParams>> PresetFinalParams = {};
			size_t DebugTypeIndex = NULL;
		public:
			GameFinalProcessing() {
				// system preset default_params.
				PresetFinalParams.push_back(std::pair("DEFAULT", SceneFinalParams()));
			}
			void GetFinalParamsPonter(SceneFinalParams* pointer) {
				FrameworkFinalParamsPtr = pointer;
			}
			void PushPresetParams(const char* params_name, const SceneFinalParams& params) {
				PresetFinalParams.push_back(std::pair(params_name, params));
			}
			// index_count: push_order, release: keep check.
			bool TYPE_PARAMS(size_t index) {
				if (index >= PresetFinalParams.size()) 
					return false;
				CurrentFinalParams = PresetFinalParams[index].second;
				return true;
			}
			// render debug_panel => "TYPE_PARAMS" invalid.
			void RenderDebugParamsPanel(const char* name);
			// preset params type,lerp.
			void RunFinalProcessing(float time_step = 1.0f);
		};
	}

	// game_view: texture => gui.
	namespace GameView {
		using TextureViewImage   = GraphicsEnginePVFX::PsagGLEngineFxImageView;
		using TextureViewCapture = GraphicsEnginePVFX::PsagGLEngineFxCaptureView;
	}

	namespace GameSyncLoader {
		// psag file_system.
		namespace FSLD = PsagLow::PsagSupLoader;
		// psag decoder / encoder channels.
		namespace DCH = PsagLow::PsagSupDCH;
		namespace ECH = PsagLow::PsagSupECH;
		// psag graphics.
		using SyncEncDecImage = PsagLow::PsagSupGraphicsOper::PsagGraphicsImageRawDat;
	}

	namespace GameNotify {
		namespace GameDataSER = ToolkitsEngineNotify::NotifySER;

		using SerRawData = ToolkitsEngineNotify::SerializeData;
		using DataMode = ToolkitsEngineNotify::MSG_DATA_MODE;

		using StationSystem = ToolkitsEngineNotify::NotifyStationSystem;
		using NotifySystem  = ToolkitsEngineNotify::NotifySystem;
	}

	namespace GameNoiseMap {
		using SharedCache = ToolkitsEngineNoise::NoiseSharedCache;

		using NoiseBlockDESC = ToolkitsEngineNoise::GenNoiseParamsDESC;
		using NoiseBlock = ToolkitsEngineNoise::GenNoiseOctavePerlin;
	}

	namespace GameMathsTools {
		float CalcDirection(const Vector2T<float>& vector);

		float           CalcFuncPointsDistance    (Vector2T<float> point0, Vector2T<float> point1);
		float           CalcFuncPointsAngle       (Vector2T<float> basic_point, Vector2T<float> point);
		Vector2T<float> CalcFuncPointAngleDistance(Vector2T<float> basic_point, float angle_deg, float distance);

		void CalcFuncLerpVec1(float* ahpla, float* target, float speed);
		void CalcFuncLerpVec2(Vector2T<float>* ahpla, Vector2T<float>* target, float speed);
		void CalcFuncLerpVec4(Vector4T<float>* ahpla, Vector4T<float>* target, float speed);

		float CalcFuncAvgDiffeVec4(const Vector4T<float>& a, const Vector4T<float>& b);

		// 引力轨道环绕, return 'force'. ChatGPT.4O, RCSZ 20241110.
		Vector2T<float> SurroundingOrbit(
			Vector2T<float> point_a, Vector2T<float> point_b, float r, float force_scale = 1.0f
		);
	}
}
#endif