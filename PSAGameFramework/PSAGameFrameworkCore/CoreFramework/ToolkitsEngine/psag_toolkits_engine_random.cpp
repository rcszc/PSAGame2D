// psag_toolkits_engine_random.
#include "psag_toolkits_engine.h"

using namespace std;
using namespace PSAG_LOGGER;

namespace ToolkitsEngineRandom {
	template<typename CHRONO_TYPE>
	int64_t GetTimeNowCount() {
		// get system_time: s, ms, us, ns.
		return chrono::duration_cast<CHRONO_TYPE>(chrono::system_clock::now().time_since_epoch()).count();
	}

	namespace Func {
		// params: max >= min, seed = 0 => default: time.
		template<typename T>
		T GenerateRandomFuncBase(T max, T min, uint64_t seed) {
			uint64_t SEED_CODE = (uint64_t)GetTimeNowCount<chrono::milliseconds>();
			if (seed != 0) SEED_CODE = seed;
			// const seed: milliseconds_time, set_value.
			mt19937_64 MtGenerator(SEED_CODE);
			uniform_real_distribution<float> Distribution((float)min, (float)max);
			return (T)Distribution(MtGenerator);
		}

		// easy generate float32 random_value.
		int32_t GenerateRandomFuncINT32(int32_t max, int32_t min, uint64_t seed) {
			if (max < min) swap(max, min);
			return GenerateRandomFuncBase<int32_t>(max, min, seed);
		}
		// easy generate signed.integer32 random_value.
		float GenerateRandomFuncFP32(float max, float min, uint64_t seed) {
			if (max < min) swap(max, min);
			return GenerateRandomFuncBase<float>(max, min, seed);
		}
	}

	void GenerateRandom1D::RandomSeedMode(RandomSeed mode) {
		switch (mode) {
		case(TimeSeedSeconds):      { GenerateSeedFunc = [&]() { return GetTimeNowCount<chrono::seconds>(); };      break; }
		case(TimeSeedMilliseconds): { GenerateSeedFunc = [&]() { return GetTimeNowCount<chrono::milliseconds>(); }; break; }
		case(TimeSeedMicroseconds): { GenerateSeedFunc = [&]() { return GetTimeNowCount<chrono::microseconds>(); }; break; }
		case(TimeSeedNanoseconds):  { GenerateSeedFunc = [&]() { return GetTimeNowCount<chrono::nanoseconds>(); };  break; }
		case(SetSeedValue):         { GenerateSeedFunc = [&]() { return RandomSeedValue; }; break; }
		default: 
			PushLogger(LogWarning, PSAGM_TOOLKITS_RAND_LABEL, "random(1d) system: seed mode invalid.");
		}
	}

	float GenerateRandom1D::CreateRandomValue(float min, float max) {
		// seed: 'GenerateSeedFunc' => MT19937 => distr.
		mt19937_64 MtGenerator((uint64_t)GenerateSeedFunc());
		uniform_real_distribution<float> Distribution(min, max);
		return Distribution(MtGenerator);
	}

	float GenerateRandom2D::CalcDistanceLength(const Vector2T<float>& point1, const Vector2T<float>& point2) {
		return sqrt(
			(point1.vector_x - point2.vector_x) * (point1.vector_x - point2.vector_x) + 
			(point1.vector_y - point2.vector_y) * (point1.vector_y - point2.vector_y)
		);
	}

	bool GenerateRandom2D::IsMinDistanceSafe(size_t number, const Vector2T<Vector2T<float>>& limit, float min_dist, float& max) {
		float RectWidth  = limit.vector_x.vector_y - limit.vector_x.vector_x;
		float RectHeight = limit.vector_y.vector_y - limit.vector_y.vector_x;
		// max points.
		float MaxPoints = (RectWidth / min_dist + 1.0f) * (RectHeight / min_dist + 1.0f);
		MaxPoints *= 0.7f; // safe-factor [20240606]
		max = MaxPoints;
		return MaxPoints > (float)number;
	}

	bool GenerateRandom2D::IsPointInRectangle(const Vector2T<float>& point, const Vector2T<float>& min, const Vector2T<float>& max) {
		return (point.vector_x >= min.vector_x && point.vector_x <= max.vector_x) &&
			(point.vector_y >= min.vector_y && point.vector_y <= max.vector_y);
	}

	bool GenerateRandom2D::CreateRandomDataset(size_t number, const Vector2T<Vector2T<float>>& limit, float min_distance) {
		if (min_distance <= 0.0f) {
			PushLogger(LogError, PSAGM_TOOLKITS_RAND_LABEL, "random(2d) system: min_distance <= 0.0f");
			return false;
		}
		if (limit.vector_x.vector_x > limit.vector_x.vector_y || limit.vector_y.vector_x > limit.vector_y.vector_y) {
			PushLogger(LogError, PSAGM_TOOLKITS_RAND_LABEL, "random(2d) system: min > max.");
			return false;
		}
		float SafeMax = 0.0f;
		// 范围内生成数量与安全稀疏度.
		if (!IsMinDistanceSafe(number, limit, min_distance, SafeMax))
			PushLogger(LogWarning, PSAGM_TOOLKITS_RAND_LABEL, "random(2d) system: number: %u safe_max: %.2f", number, SafeMax);
		
		// generate 2d points.
		while (RandomCoordGroup.size() < number) {
			Vector2T<float> NewPoint = Vector2T<float>(
				CreateRandomValue(limit.vector_x.vector_x, limit.vector_x.vector_y), 
				CreateRandomValue(limit.vector_y.vector_x, limit.vector_y.vector_y)
			);

			bool PointIsValid = true;
			for (const auto& Point : RandomCoordGroup) {
				if (CalcDistanceLength(NewPoint, Point) < min_distance) {
					PointIsValid = false;
					break;
				}
			}
			if (PointIsValid)
				RandomCoordGroup.push_back(NewPoint);
		}
		PushLogger(LogInfo, PSAGM_TOOLKITS_RAND_LABEL, "random(2d) system: create: %u", number);
		return true;
	}

	bool GenerateRandom2D::DatasetCropCircle(const Vector2T<float>& center, float radius, bool flag) {
		if (RandomCoordGroup.empty()) {
			PushLogger(LogWarning, PSAGM_TOOLKITS_RAND_LABEL, "random(2d) system: crop: dataset empty.");
			return false;
		}
		if (radius <= 0.0f) {
			PushLogger(LogError, PSAGM_TOOLKITS_RAND_LABEL, "random(2d) system: crop: radius <= 0.0f");
			return false;
		}
		// point > circle_radius => erase.
		auto it = partition(RandomCoordGroup.begin(), RandomCoordGroup.end(), 
			[&](Vector2T<float> value) {
				float DisLength = CalcDistanceLength(value, center);
				// mode type: true: swap value.
				if (flag) swap(DisLength, radius);
				return !(DisLength < radius);
			}
		);
		size_t CropPointsNumber = RandomCoordGroup.size();
		RandomCoordGroup.erase(it, RandomCoordGroup.end());
		
		CropPointsNumber -= RandomCoordGroup.size();
		PushLogger(LogInfo, PSAGM_TOOLKITS_RAND_LABEL, "random(2d) system: crop(circle): %u",
			CropPointsNumber);
		return true;
	}

	bool GenerateRandom2D::DatasetCropRectangle(const Vector2T<float>& min_point, const Vector2T<float>& max_point) {
		if (RandomCoordGroup.empty()) {
			PushLogger(LogWarning, PSAGM_TOOLKITS_RAND_LABEL, "random(2d) system: crop: dataset empty.");
			return false;
		}
		if (min_point.vector_x > max_point.vector_x || min_point.vector_y > max_point.vector_y) {
			PushLogger(LogError, PSAGM_TOOLKITS_RAND_LABEL, "random(2d) system: crop: rect_min > rect_max.");
			return false;
		}
		size_t CropPointsNumber = RandomCoordGroup.size();
		// point > rect_border => erase.
		for (auto it = RandomCoordGroup.begin(); it != RandomCoordGroup.end(); ++it) {
			if (IsPointInRectangle(*it, min_point, max_point))
				RandomCoordGroup.erase(it);
		}
		CropPointsNumber -= RandomCoordGroup.size();
		PushLogger(LogInfo, PSAGM_TOOLKITS_RAND_LABEL, "random(2d) system: crop(rect): %u", CropPointsNumber);
		return true;
	}
}