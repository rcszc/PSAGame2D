// particle_generator_points.
#include "../psag_graphics_engine.h"

using namespace std;
using namespace PSAG_LOGGER;

namespace GraphicsEngineParticle {
	bool GeneratorPointsDiffu::ConfigCreateNumber(float number) {
		// generator particles >= 1.
		if (number < 1.0f) return false;
		ParticlesNumber = (size_t)number;
		return true;
	}

	void GeneratorPointsDiffu::ConfigLifeDispersion(const Vector2T<float>& rand_life) {
		// particle rand life: [0.0,n].
		float LifeMin = rand_life.vector_x <= 0.0f ? 0.0f : rand_life.vector_x;
		float LifeMax = rand_life.vector_y <= 0.0f ? 0.0f : rand_life.vector_y;

		RandomLimitLife = Vector2T<float>(LifeMin, LifeMax);
	}

	void GeneratorPointsDiffu::ConfigSizeDispersion(const Vector2T<float>& rand_size) {
		// particle rand size: [0.0,n].
		float SizeMin = rand_size.vector_x <= 0.0f ? 0.0f : rand_size.vector_x;
		float SizeMax = rand_size.vector_y <= 0.0f ? 0.0f : rand_size.vector_y;

		RandomScaleSize = Vector2T<float>(SizeMin, SizeMax);
	}

	void GeneratorPointsDiffu::ConfigGenPos(const Vector2T<float>& position) {
		// render position layer: const 2.5f, 20250119 RCSZ.
		OffsetPosition = Vector3T<float>(position.vector_x, position.vector_y, 2.5f);
	}
	void GeneratorPointsDiffu::ConfigGenPosRand(const Vector2T<float>& position_rand) {
		RandomPosition = position_rand;
	}
	void GeneratorPointsDiffu::ConfigGenVector(const Vector2T<float>& speed) {
		RandomSpeed = speed;
	}

	void GeneratorPointsDiffu::ConfigRandomColorSystem(
		const Vector2T<float>& r, const Vector2T<float>& g, const Vector2T<float>& b,
		ColorChannelMode mode
	) {
		RandomColorSystem = __COLOR_SYSTEM_TYPE(r, g, b, mode, &EnableGrayscale);
	}
}