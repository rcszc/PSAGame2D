// psag_toolkits_engine_noise. RCSZ. [middle_level_engine]
// toolkits(module) oct_noise_map engine. [20250211]
// only-header 八度柏林噪声生成.

#ifndef __PSAG_TOOLKITS_ENGINE_NOISE_H
#define __PSAG_TOOLKITS_ENGINE_NOISE_H

#define ENABLE_LOWMODULE_AUDIO
#include "../psag_lowlevel_support.h"

namespace ToolkitsEngineNoise {
	StaticStrLABEL PSAGM_TOOLKITS_NOISE_LABEL = "PSAG_TOOL_NOISE";

	using NoiseGradient = std::pair<double, double>;
	using NoiseKey      = std::array<int64_t, 2>;
	// noise shared cache key hash.
	struct ArrayHash {
		template <typename T>
		size_t operator ()(const std::array<T, 2>& v) const {
			size_t H0 = std::hash<T>{}(v[0]);
			size_t H1 = std::hash<T>{}(v[1]);
			return H0 ^ (H1 << 1);
		}
	};
	// thread safe shared cache.
	struct NoiseSharedCache {
		// key: noise vector x,y(array_2), value: noise gradient.
		std::unordered_map<NoiseKey, NoiseGradient, ArrayHash> Cache = {};
		std::shared_mutex CacheMutex = {};
	};
	// generate noise map config.
	struct GenNoiseParamsDESC {
		NoiseSharedCache* CacheIndex = nullptr;
		size_t BlockNumberGrids = 16;

		size_t NoiseOctaves     = 1;   // 倍频数
		double NoisePersistence = 0.6; // 振幅衰减
		double NoiseLacunarity  = 1.2; // 频率增长倍率
		double NoiseScale       = 1.2; // 缩放因子

		bool VALIDATE() const {
			// check grids number, num: n^2.
			size_t GridsLength = (size_t)std::sqrt(BlockNumberGrids);
			if ((size_t)std::pow(GridsLength, 2) != BlockNumberGrids)
				return false;
			// check noise params.
			if (NoiseOctaves < 1)        return false;
			if (NoisePersistence <= 0.0) return false;
			if (NoiseLacunarity < 1.0)   return false;
			if (NoiseScale <= 0.0001)    return false;
			return true;
		}
	};

	// t-safe, octave perlin noise map generate.
	// warn: high calc & memory, 20250204 RCSZ.
	class GenNoiseOctavePerlin {
	protected:
		static double MathsFade(double t) {
			return t * t * t * (t * (t * 6 - 15) + 10);
		}
		static double MathsLerp(double a, double b, double t) {
			return a + t * (b - a);
		}
		// get format time now count value.
		template<typename time_type>
		static int64_t GetTimeNowCount() {
			// get system_time: s, ms, us, ns.
			return std::chrono::duration_cast<time_type>(
				std::chrono::system_clock::now().time_since_epoch()
			).count();
		}
		static uint8_t FMT_PIXEL(double value) {
			return uint8_t(value * 255.0);
		}
		// noise block params.
		int64_t BlockSize = 0, BlockGridSpacing = 0;
		int64_t Xquadrant = 0, Yquadrant = 0;

		GenNoiseParamsDESC GenNoiseConfig = {};
		NoiseSharedCache* ResourceIndex = nullptr;

		// warn: high overhead. 20250210 RCSZ.
		NoiseGradient GradientGenerateMap(NoiseKey key) {
			// shared mutex check & read cache.
			std::shared_lock<std::shared_mutex> SLock(GenNoiseConfig.CacheIndex->CacheMutex);
			auto it = GenNoiseConfig.CacheIndex->Cache.find(key);
			if (it != GenNoiseConfig.CacheIndex->Cache.end()) {
				// shared vector data cache hit.
				return it->second;
			}
			SLock.unlock();
			// generate 0(0pi) - 360(2pi) deg vector.
			std::mt19937_64 MtGenerator(GetTimeNowCount<std::chrono::nanoseconds>());
			std::uniform_real_distribution<double> Distribution(0.0, 2.0 * PSAG_M_PI);
			// 2d angle vector.
			double Angle = Distribution(MtGenerator);
			NoiseGradient ResultNoiseGradient = { cos(Angle), sin(Angle) };
			// unique mutex write cache.
			std::unique_lock<std::shared_mutex> ULock(GenNoiseConfig.CacheIndex->CacheMutex);
			GenNoiseConfig.CacheIndex->Cache.emplace(key, ResultNoiseGradient);
			ULock.unlock();
			return ResultNoiseGradient;
		}

		double GradientGrid(int64_t ix, int64_t iy, double x, double y) {
			int64_t Gx = ix / BlockGridSpacing;
			int64_t Gy = iy / BlockGridSpacing;
			// clac hash key_params. 2025_02_10 RCSZ.
			NoiseKey HashKey = { Gx, Gy };
			auto [GxValue, GyValue] = GradientGenerateMap(HashKey);

			double Dx = (x - (double)ix) / (double)BlockGridSpacing;
			double Dy = (y - (double)iy) / (double)BlockGridSpacing;
			return Dx * GxValue + Dy * GyValue;
		}

		double CalcPerlin(double x, double y) {
			// grid index calculate.
			int64_t x0 = ((int64_t)floor(x) / BlockGridSpacing) * BlockGridSpacing;
			int64_t y0 = ((int64_t)floor(y) / BlockGridSpacing) * BlockGridSpacing;
			int64_t x1 = x0 + BlockGridSpacing;
			int64_t y1 = y0 + BlockGridSpacing;

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
		
		double CalcOctavePerlin(double x, double y) {
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
	public:
		GenNoiseOctavePerlin(
			int64_t rect_size, int64_t quad_x, int64_t quad_y,
			const GenNoiseParamsDESC& config
		) : BlockSize(rect_size), Xquadrant(quad_x), Yquadrant(quad_y),
			GenNoiseConfig(config)
		{
			// check cache_index & params status.
			if (GenNoiseConfig.CacheIndex == nullptr)
				PSAG_LOGGER::PushLogger(LogError, PSAGM_TOOLKITS_NOISE_LABEL, "noise shared cache nullptr.");
			if (!GenNoiseConfig.VALIDATE())
				PSAG_LOGGER::PushLogger(LogError, PSAGM_TOOLKITS_NOISE_LABEL, "noise params invalid.");
			// calc: block grids spacing. (rect_length) 
			BlockGridSpacing = BlockSize / (int64_t)sqrt(GenNoiseConfig.BlockNumberGrids);
			PSAG_LOGGER::PushLogger(LogInfo, PSAGM_TOOLKITS_NOISE_LABEL, "noise config complete.");
		}

		// generate noise map dataset, array.
		template<typename T>
		std::vector<T> GenerateNoiseData(
			const std::function<T(std::array<double, 3>, bool*)>& CAB_FUNC
		) {
			double OffesetCoord[2] = {
				double(Xquadrant * BlockSize), double(Yquadrant * BlockSize)
			};
			// start noise clac timer.
			auto BeginTime = std::chrono::system_clock::now();
			// n x n, data 1d size: block_size ^ 2.
			size_t DataLength = (size_t)std::pow(BlockSize, 2);
			std::vector<T> NoiseMapData = {};
			// generate noise coords map.
			for (int64_t y = 0; y < BlockSize; ++y) {
				for (int64_t x = 0; x < BlockSize; ++x) {
					// calcuate coord & scale.
					double Coordx = (OffesetCoord[0] + (double)x) * GenNoiseConfig.NoiseScale;
					double Coordy = (OffesetCoord[1] + (double)y) * GenNoiseConfig.NoiseScale;
					bool DataIsValid = false;
					// calcuate oct perlin, proc data.
					T FilterResult = CAB_FUNC({
						Coordx, Coordy, // data format v20250208.
						abs(CalcOctavePerlin(Coordx, Coordy))
						}, &DataIsValid
					);
					// valid data => add result dataset.
					if (DataIsValid) NoiseMapData.push_back(FilterResult);
				}
			}
			// print params: clac time(ms), size, memory.
			PSAG_LOGGER::PushLogger(LogInfo, PSAGM_TOOLKITS_NOISE_LABEL,
				"noise map calc time: %.2f ms",
				(float)std::chrono::duration_cast<std::chrono::microseconds>(
					std::chrono::system_clock::now() - BeginTime
				).count() * 0.001f
			);
			double ResultDataSizeKib =
				double(NoiseMapData.size() * sizeof(T)) * 0.0009766;
			PSAG_LOGGER::PushLogger(LogInfo, PSAGM_TOOLKITS_NOISE_LABEL,
				"noise map valid size: %u / %u, mem: %.2f kib",
				NoiseMapData.size(), DataLength, (float)ResultDataSizeKib
			);
			return NoiseMapData;
		}

		// data std format: [x, y, value] 2025_02_10 RCSZ.
		// default data filter mode.
		using DefaultFmt = std::array<double, 3>;
		std::vector<std::array<double, 3>> GenerateNoiseDataSTD() {
			// default filter callback. 2025_02_10 RCSZ.
			auto Filter = [](std::array<double, 3> values, bool* valueflag) {
				*valueflag = true; return values;
			};
			return GenerateNoiseData<DefaultFmt>(Filter);
		}

		ImageRawData GenerateNoiseDataImage() {
			// default pixels callback RGB888. 2025_02_10 RCSZ.
			auto PixelsData = [](std::array<double, 3> values, bool* valueflag) {
				// double format [0,255] pixel.
				*valueflag = true; return std::array<uint8_t, 3>{
					FMT_PIXEL(values[2]), FMT_PIXEL(values[2]), FMT_PIXEL(values[2])
				};
			};
			auto Dataset = GenerateNoiseData<std::array<uint8_t, 3>>(PixelsData);
			// noise map raw data => rgb888 image.
			RawDataStream ImagePixels = {};
			ImageRawData ResultImage = {};

			for (const auto& Pixel : Dataset)
				ImagePixels.insert(ImagePixels.end(), Pixel.begin(), Pixel.end());
			ResultImage.ImagePixels = ImagePixels;

			// free temp raw data.
			Dataset.clear(); Dataset.shrink_to_fit();
			ImagePixels.clear(); ImagePixels.shrink_to_fit();

			ResultImage.Channels = 3;
			ResultImage.Width  = (uint32_t)BlockSize;
			ResultImage.Height = (uint32_t)BlockSize;
			return ResultImage;
		}
	};
}
#endif