// psag_collect_engine. RCSZ. [middle_level_engine]
// collection(module) engine. [20240606]
// random, sound, filesystem, thread.

#ifndef __PSAG_COLLECT_ENGINE_H
#define __PSAG_COLLECT_ENGINE_H

#define ENABLE_LOWMODULE_FILESYS
#define ENABLE_LOWMODULE_SOUND
#define ENABLE_LOWMODULE_THREAD
#include "../psag_lowlevel_support.h"

class __COLLECT_ENGINE_TIMESETP {
protected:
	static float CollectEngineTimeStep;
};

namespace CollectEngineRandom {
	StaticStrLABEL PSAGM_COLENGINE_RAND_LABEL = "PSAG_COLL_RADNOM";
	namespace Func {
		float GenerateRandomFunc(float max, float min, uint64_t seed = 0);
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

namespace CollectEnginePawn {
	StaticStrLABEL PSAGM_COLENGINE_PAWN_LABEL = "PSAG_COLL_PAWN";

	class GamePlayerPawn :public __COLLECT_ENGINE_TIMESETP {
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

namespace CollectEngineCamera {
	StaticStrLABEL PSAGM_COLENGINE_CAMERA_LABEL = "PSAG_COLL_CAMERA";

	class GamePlayerComaeraMP :public __COLLECT_ENGINE_TIMESETP {
	protected:
		Vector2T<float> CameraPositionTarget = {};
		Vector2T<float> CameraPosition       = {};

		Vector2T<float> WindowRectRange  = {};
		Vector2T<float> WindowResolution = {};

		float CALC_LERP_SCALE = 1.0f;
	public:
		GamePlayerComaeraMP(
			const Vector2T<float>& rect_range, const Vector2T<uint32_t>& window_size, 
			float hardness
		);
		void PlayerCameraRun(const Vector2T<float>& window_coord, const Vector2T<float>& actor_speed);
		// calc_target => calc_position, out_pos = pos + offset.
		Vector2T<float> GetCameraPosition(const Vector2T<float>& camera_offset = Vector2T<float>());
	};
}

namespace CollectEngineTimerClock {
	StaticStrLABEL PSAGM_COLENGINE_TIMER_LABEL = "PSAG_COLL_TIMER";

	// timer accuracy: us, setting: ms.
	class GameCycleTimer {
	protected:
		std::chrono::system_clock::time_point TimeTempTarget = std::chrono::system_clock::now();
	public:
		bool CycleTimerFlagGet();
		bool CycleTimerClearReset(float time_delay);

		// return: milliseconds.
		float GetTimeNowCount();
	};
}

#endif