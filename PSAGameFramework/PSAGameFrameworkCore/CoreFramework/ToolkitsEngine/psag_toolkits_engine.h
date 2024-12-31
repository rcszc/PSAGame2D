// psag_toolkits_engine. RCSZ. [middle_level_engine]
// toolkits(module) engine. [20240606]
// random, sound, filesystem, thread.

#ifndef __PSAG_TOOLKITS_ENGINE_H
#define __PSAG_TOOLKITS_ENGINE_H

#define ENABLE_LOWMODULE_AUDIO
#include "../psag_lowlevel_support.h"
#include "psag_toolkits_engine_notify.hpp"

// toolkits_engine timestep, mainevent => TE.
class __TOOLKITS_ENGINE_TIMESETP {
protected:
	static float ToolkitsEngineTimeStep;
};

namespace ToolkitsEngineRandom {
	StaticStrLABEL PSAGM_TOOLKITS_RAND_LABEL = "PSAG_TOOL_RADNOM";

	namespace Func {
		int32_t GenerateRandomFuncINT32(int32_t max, int32_t min, uint64_t seed = 0);
		float   GenerateRandomFuncFP32(float max, float min, uint64_t seed = 0);
	}

	enum RandomSeed {
		TimeSeedSeconds      = 1 << 1,
		TimeSeedMilliseconds = 1 << 2,
		TimeSeedMicroseconds = 1 << 3,
		TimeSeedNanoseconds  = 1 << 4,
		// using user settings.
		SetSeedValue = 1 << 5
	};

	class GenerateRandom1D {
	protected:
		std::function<int64_t(void)> GenerateSeedFunc = [&]() { return RandomSeedValue; };
	public:
		// default mode: 'SetSeedValue'
		void RandomSeedMode(RandomSeed mode);
		// user set seed_value(int64_t)
		int64_t RandomSeedValue = NULL;

		float CreateRandomValue(float min, float max);
	};

	class GenerateRandom2D :public GenerateRandom1D {
	protected:
		float CalcDistanceLength(const Vector2T<float>& point1, const Vector2T<float>& point2);

		bool IsMinDistanceSafe(size_t number, const Vector2T<Vector2T<float>>& limit, float min_dist, float& max);
		bool IsPointInRectangle(const Vector2T<float>& point, const Vector2T<float>& min, const Vector2T<float>& max);
	public:
		// result coord_array cache.
		std::vector<Vector2T<float>> RandomCoordGroup = {};
		
		// limit: x.x: x.min, x.y: x.max, y.x: y.min, y.y: y.max.
		bool CreateRandomDataset(size_t number, const Vector2T<Vector2T<float>>& limit, float min_distance);

		bool DatasetCropCircle(const Vector2T<float>& center, float radius, bool flag = false);
		bool DatasetCropRectangle(const Vector2T<float>& min_point, const Vector2T<float>& max_point);
	};
}

namespace ToolkitsEnginePawn {
	StaticStrLABEL PSAGM_TOOLKITS_PAWN_LABEL = "PSAG_TOOL_PAWN";

	class GamePlayerPawn :public __TOOLKITS_ENGINE_TIMESETP {
	protected:
		// imgui keyboard unique mappings.
		static std::unordered_map<ImGuiKey, Vector2T<float>> KeyboardMappings;

		Vector2T<float> DampingEffectVector = {};
		Vector2T<float> MoveSpeedTarget = {};
	public:
		GamePlayerPawn(const Vector2T<float>& damping);

		bool MouseButtonPressed_R(bool pulse = false);
		bool MouseButtonPressed_L(bool pulse = false);

		bool KeyboardPressed_R();
		bool KeyboardPressed_F();

		// control(value): x,y[+-].
		Vector2T<float> ControlMoveVector    = {};
		Vector2T<float> ControlMousePosition = {};

		void PlayerPawnRun(float speed_value = 1.0f);
	};
}

namespace ToolkitsEngineCamera {
	StaticStrLABEL PSAGM_TOOLKITS_CAMERA_LABEL = "PSAG_TOOL_CAMERA";

	class GamePlayerCameraMP :public __TOOLKITS_ENGINE_TIMESETP {
	protected:
		Vector2T<float> CameraPositionTarget = {};
		Vector2T<float> CameraPosition       = {};

		Vector2T<float> WindowRectRange  = {};
		Vector2T<float> WindowResolution = {};

		float CALC_LERP_SCALE = 1.0f;
	public:
		GamePlayerCameraMP(
			const Vector2T<float>& rect_range, const Vector2T<uint32_t>& window_size, 
			float hardness
		);
		void PlayerCameraLerpValue(float value);
		// camera system_run.
		void PlayerCameraRun(const Vector2T<float>& window_coord, const Vector2T<float>& actor_speed);
		void PlayerCameraRunFixed(const Vector2T<float>& actor_position);

		// calc_target => calc_position, out_pos = pos + offset.
		Vector2T<float> GetCameraPosition(const Vector2T<float>& camera_offset = Vector2T<float>());
	};

	class GamePlayerCameraGM :public __TOOLKITS_ENGINE_TIMESETP {
	protected:
		Vector2T<Vector2T<float>> CameraPositionRectLimit = {};

		Vector2T<float> CameraPositionTarget = {};
		Vector2T<float> CameraPosition       = {};

		float CALC_LERP_SCALE = 1.0f;
	public:
		GamePlayerCameraGM(const Vector2T<float>& map_limit_min, const Vector2T<float>& map_limit_max);

		void PlayerCameraLerpValue(float value);
		// camera system_run. speed delta(scale) default = 1.0f
		void PlayerCameraRun(const Vector2T<float>& mouse_vec_speed, float speed_scale = 1.0f);

		// calc_target => calc_position, out_pos = pos + offset.
		Vector2T<float> GetCameraPosition(const Vector2T<float>& camera_offset = Vector2T<float>());
	};
}

namespace ToolkitsEngineTimerClock {
	StaticStrLABEL PSAGM_TOOLKITS_TIMER_LABEL = "PSAG_TOOL_TIMER";

	// timer accuracy: us, setting: ms.
	class GameCycleTimer {
	protected:
		std::chrono::system_clock::time_point TimeTempTarget = std::chrono::system_clock::now();
	public:
		bool CycleTimerGetFlag();
		bool CycleTimerClearReset(float time_delay);

		// now time_point return: milliseconds.
		float GetTimeNowCount();
	};
}

namespace ToolkitsEngineSound {
	StaticStrLABEL PSAGM_TOOLKITS_SOUND = "PSAG_TOOL_SOUND";

	// framework low => manager.
	class GamePlayerSound :public PsagLow::PsagSupAudioLLRES {
	protected:
		PsagLow::PsagSupAudioDataPlayer* AudioPlayer = nullptr;
		ResUnique RawStreamResource = {};

	public:
		GamePlayerSound(const RawAudioStream& data);
		~GamePlayerSound();

		PsagLow::PsagSupAudioDataPlayer* AudioPlayerOperate();
	};
}

// imgui gui_extension & gui_component version 0.1
namespace ToolkitsEngineGuiExt {
	StaticStrLABEL PSAGM_TOOLKITS_GUI_EXT = "PSAG_TOOL_GUI_EXT";

	void BoxText(const char* text, float box_length, const ImVec4& color = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));

	namespace Large {
		// sampler_2d points array.
		using PlotsDatasetSRC = std::vector<std::vector<float>>;
		using PlotsDatasetPTR = PlotsDatasetSRC*;

		class ImMegaPlotsDataView {
		protected:
			ImVec4 ViewWindowColorsystem = {};
			ImVec2 ViewWindowSize        = {};
			ImVec2 ViewWindowYCoordLimit = ImVec2(0.0f, 3000.0f);

			std::vector<ImVec4> DrawPlotColors = { ImVec4(0.0f, 1.0f, 1.0f, 1.0f) };

			float DataSampleRate = 1.0f;

			float ViewWindowDwWidthPlots    = 2.5f;
			float ViewWindowDwWidthRuler    = 0.0f;
			float ViewWindowDrawLength      = 0.0f;
			float ViewWindowScrollxPosition = 0.0f;

			bool SettingWindowFlag = false;

			PlotsDatasetSRC SampleDatasetPoints = {};
			// render: ruler_child_window & view_child_window, [MUL-PLOT].
			void CoreViewWindowRender(PlotsDatasetPTR data);
			void SettingWindiwRender(bool* window_open);
		public:
			ImMegaPlotsDataView();
			~ImMegaPlotsDataView() {
				PSAG_LOGGER::PushLogger(LogInfo, PSAGM_TOOLKITS_GUI_EXT, "plot_comp: object: ptr[%x] delete.", this);
			}

			PlotsDatasetPTR GetPlotsDatasetRef() {
				return &SampleDatasetPoints;
			}
			void DrawImGuiDataPlot(const std::string& window_name, bool fixed = true);
		};
	}
}

#endif