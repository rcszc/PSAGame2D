// psag_toolkits_engine_random.
#include "psag_toolkits_engine.h"

using namespace std;
using namespace PSAG_LOGGER;

namespace ToolkitsEngineRandom {
	template<typename CHRONO_TYPE>
	int64_t GetTimeNowCount() {
		// get system_time: s, ms, us, ns.
		return chrono::duration_cast<CHRONO_TYPE>(
			chrono::system_clock::now().time_since_epoch()
		).count();
	}
	// easy generate random func.
	namespace Func {
		// params: max >= min, seed = 0 => default: time.
		template<typename T>
		T GenerateRandomFuncBase(T min, T max, uint64_t seed) {
			uint64_t SEED_CODE = (uint64_t)GetTimeNowCount<chrono::nanoseconds>();
			if (seed != 0) SEED_CODE = seed;
			// const seed: milliseconds_time, set_value.
			mt19937_64 MtGenerator(SEED_CODE);
			uniform_real_distribution<float> Distribution((float)min, (float)max);
			return (T)Distribution(MtGenerator);
		}

		// easy generate float32 random_value.
		int32_t GenerateRandomFuncINT32(int32_t min, int32_t max, uint64_t seed) {
			if (max < min) swap(max, min);
			return GenerateRandomFuncBase<int32_t>(min, max, seed);
		}
		// easy generate signed.integer32 random_value.
		float GenerateRandomFuncFP32(float min, float max, uint64_t seed) {
			if (max < min) swap(max, min);
			return GenerateRandomFuncBase<float>(min, max, seed);
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

	bool GenerateRandom2D::IsMinDistanceSafe(
		size_t number, const Vector2T<Vector2T<float>>& limit, float min_dist, float& max
	) {
		float RectWidth  = limit.vector_x.vector_y - limit.vector_x.vector_x;
		float RectHeight = limit.vector_y.vector_y - limit.vector_y.vector_x;
		// max points.
		float MaxPoints = (RectWidth / min_dist + 1.0f) * (RectHeight / min_dist + 1.0f);
		MaxPoints *= 0.7f; // safe-factor [20240606]
		max = MaxPoints;
		return MaxPoints > (float)number;
	}

	bool GenerateRandom2D::IsPointInRectangle(
		const Vector2T<float>& point, const Vector2T<float>& min, const Vector2T<float>& max
	) {
		return (point.vector_x >= min.vector_x && point.vector_x <= max.vector_x) &&
			(point.vector_y >= min.vector_y && point.vector_y <= max.vector_y);
	}

	bool GenerateRandom2D::CreateRandomDataset(
		size_t number, const Vector2T<Vector2T<float>>& limit, float min_distance
	) {
		if (min_distance <= 0.0f) {
			PushLogger(LogError, PSAGM_TOOLKITS_RAND_LABEL, "random(2d) system: min_distance <= 0.0f");
			return false;
		}
		if (limit.vector_x.vector_x > limit.vector_x.vector_y || 
			limit.vector_y.vector_x > limit.vector_y.vector_y
		) {
			PushLogger(LogError, PSAGM_TOOLKITS_RAND_LABEL, "random(2d) system: min > max.");
			return false;
		}
		float SafeMax = 0.0f;
		// 范围内生成数量与安全稀疏度.
		if (!IsMinDistanceSafe(number, limit, min_distance, SafeMax)) {
			PushLogger(LogWarning, PSAGM_TOOLKITS_RAND_LABEL, "random(2d) system: number: %u safe: %.2f",
				number, SafeMax);
		}
		// generate 2d points.
		while (RandomCoordGroup.size() < number) {
			Vector2T<float> NewPoint = Vector2T<float>(
				CreateRandomValue(limit.vector_x.vector_x, limit.vector_x.vector_y), 
				CreateRandomValue(limit.vector_y.vector_x, limit.vector_y.vector_y)
			);
			// check random point status.
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

	double GenNoiseOctavePerlin::MathsFade(double t) {
		return t * t * t * (t * (t * 6 - 15) + 10);
	}
	double GenNoiseOctavePerlin::MathsLerp(double a, double b, double t) {
		return a + t * (b - a);
	}

	NoiseGradient GenNoiseOctavePerlin::GradientGenerateMap(uint64_t key) {
		NoiseGradient ResultNoiseGradient = {};
		bool StatusFlag = false;
		{
			shared_lock<shared_mutex> Lock(GenNoiseConfig.CacheIndex->CacheMutex);
			auto it = GenNoiseConfig.CacheIndex->Cache.find(key);
			if (it != GenNoiseConfig.CacheIndex->Cache.end()) return GenNoiseConfig.CacheIndex->Cache[key];
			else StatusFlag = true;
		}
		if (StatusFlag) {
			// generate 0(0pi) - 360(2pi) deg vector.
			mt19937_64 MtGenerator(GetTimeNowCount<chrono::nanoseconds>());
			uniform_real_distribution<double> Distribution(0.0, 2.0 * PSAG_M_PI);
			// 2d angle vector.
			double Angle = Distribution(MtGenerator);
			// thread safe write cache.
			unique_lock<shared_mutex> Lock(GenNoiseConfig.CacheIndex->CacheMutex);
			GenNoiseConfig.CacheIndex->Cache[key] 
				= ResultNoiseGradient = { cos(Angle), sin(Angle) };
			Lock.unlock();
		}
		return ResultNoiseGradient;
	}

	double GenNoiseOctavePerlin::GradientGrid(size_t ix, size_t iy, double x, double y) {
		size_t Gx = ix / BlockGridSpacing;
		size_t Gy = iy / BlockGridSpacing;
		// clac params: const value update: 2025_01_05 RCSZ.
		uint64_t HashKey = (uint64_t)Gx * 73856093 ^ (uint64_t)Gy * 19349663;
		auto [GxValue, GyValue] = GradientGenerateMap(HashKey);

		double Dx = (x - (double)ix) / (double)BlockGridSpacing;
		double Dy = (y - (double)iy) / (double)BlockGridSpacing;
		return Dx * GxValue + Dy * GyValue;
	}

	double GenNoiseOctavePerlin::CalcPerlin(double x, double y) {
		// grid index calculate.
		size_t x0 = ((size_t)floor(x) / BlockGridSpacing) * BlockGridSpacing;
		size_t y0 = ((size_t)floor(y) / BlockGridSpacing) * BlockGridSpacing;
		size_t x1 = x0 + BlockGridSpacing;
		size_t y1 = y0 + BlockGridSpacing;

		double sx = MathsFade((x - (double)x0) / (double)BlockGridSpacing);
		double sy = MathsFade((y - (double)y0) / (double)BlockGridSpacing);

		double An0 = GradientGrid(x0, y0, x, y);
		double An1 = GradientGrid(x1, y0, x, y);

		double Bn0 = GradientGrid(x0, y1, x, y);
		double Bn1 = GradientGrid(x1, y1, x, y);

		double ix0 = MathsLerp(An0, An1, sx);
		double ix1 = MathsLerp(Bn0, Bn1, sx);
		return MathsLerp(ix0, ix1, sy);
	}

	double GenNoiseOctavePerlin::CalcOctavePerlin(double x, double y) {
		double ValueCount = 0.0, ValueMax = 0.0;
		double InitFrequency = 1.0;
		double InitAmplitude = 1.0;

		for (size_t i = 0; i < GenNoiseConfig.NoiseOctaves; ++i) {
			ValueCount += CalcPerlin(x * InitFrequency, y * InitFrequency) * InitAmplitude;
			ValueMax   += InitAmplitude;
			// amp,freq params.
			InitAmplitude *= GenNoiseConfig.NoisePersistence;
			InitFrequency *= GenNoiseConfig.NoiseLacunarity;
		}
		return ValueCount / ValueMax;
	}

	GenNoiseOctavePerlin::GenNoiseOctavePerlin(
		size_t rect_size, size_t quad_x, size_t quad_y, const GenNoiseParamsDESC& config
	) : BlockSize(rect_size), QuadrantX(quad_x), QuadrantY(quad_y), GenNoiseConfig(config) {
		// check cache_index & params status.
		if (GenNoiseConfig.CacheIndex == nullptr)
			PSAG_LOGGER::PushLogger(LogError, PSAGM_TOOLKITS_RAND_LABEL, "noise shared cache nullptr.");
		if (!GenNoiseConfig.VALIDATE())
			PSAG_LOGGER::PushLogger(LogError, PSAGM_TOOLKITS_RAND_LABEL, "noise params invalid.");
		// calc: block grids spacing. (rect_length) 
		BlockGridSpacing = BlockSize / (size_t)sqrt(GenNoiseConfig.BlockNumberGrids);
	}

	ImageRawData GenNoiseOctavePerlin::GenGrayscaleImage(
		function<double(size_t, size_t, double)> FILTER_FUNC
	) {
		ImageRawData ImagePixels = {};
		ImagePixels.ImagePixels.resize(BlockSize * BlockSize * 3);
		// setting image params.
		ImagePixels.Channels = 3;
		ImagePixels.Width    = (uint32_t)BlockSize;
		ImagePixels.Height   = (uint32_t)BlockSize;
		// generate noise image pixels.
		for (size_t y = 0; y < BlockSize; ++y) {
			for (size_t x = 0; x < BlockSize; ++x) {
				// calc coord & scale.
				double Coordx = (QuadrantX * BlockSize + x) * GenNoiseConfig.NoiseScale;
				double Coordy = (QuadrantY * BlockSize + y) * GenNoiseConfig.NoiseScale;

				uint8_t FormatValue = uint8_t(FILTER_FUNC(
					x, y, abs(CalcOctavePerlin(Coordx, Coordy)) * 255.0)
				);
				size_t index = (y * BlockSize + x) * 3;
				// rgb888 grayscale format pixel.
				ImagePixels.ImagePixels[index + 0] = FormatValue;
				ImagePixels.ImagePixels[index + 1] = FormatValue;
				ImagePixels.ImagePixels[index + 2] = FormatValue;
			}
		}
		return ImagePixels;
	}
}