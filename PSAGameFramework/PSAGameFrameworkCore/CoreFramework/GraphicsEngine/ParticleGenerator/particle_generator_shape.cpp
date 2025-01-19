// particle_generator_shape.
#include "../psag_graphics_engine.h"

using namespace std;
using namespace PSAG_LOGGER;

namespace GraphicsEngineParticle {
	bool GeneratorShape::ConfigCreateNumber(float number) {
		// generator particles >= 8.
		if (number < 8.0f) return false;
		ParticlesNumber = (size_t)number;
		return true;
	}

	void GeneratorShape::ConfigLifeDispersion(const Vector2T<float>& rand_life) {
		// particle rand life: [0.0,n].
		float LifeMin = rand_life.vector_x <= 0.0f ? 0.0f : rand_life.vector_x;
		float LifeMax = rand_life.vector_y <= 0.0f ? 0.0f : rand_life.vector_y;

		RandomLimitLife = Vector2T<float>(LifeMin, LifeMax);
	}

	void GeneratorShape::ConfigSizeDispersion(const Vector2T<float>& rand_size) {
		// particle rand size: [0.0,n].
		float SizeMin = rand_size.vector_x <= 0.0f ? 0.0f : rand_size.vector_x;
		float SizeMax = rand_size.vector_y <= 0.0f ? 0.0f : rand_size.vector_y;

		RandomScaleSize = Vector2T<float>(SizeMin, SizeMax);
	}

	void GeneratorShape::ConfigGenPos(const Vector2T<float>& position) {
		// render position layer: const 2.5f, 20250119 RCSZ.
		OffsetPosition = Vector3T<float>(position.vector_x, position.vector_y, 2.5f);
	}
	void GeneratorShape::ConfigGenPosRand(const Vector2T<float>& position_rand) {
		RandomPosition = position_rand;
	}
	void GeneratorShape::ConfigGenVector(const Vector2T<float>& speed) {
		RandomSpeed = speed;
	}

	void GeneratorShape::ConfigRandomColorSystem(
		const Vector2T<float>& r, const Vector2T<float>& g, const Vector2T<float>& b,
		ParticlesGenMode::ColorChannelMode mode
	) {
		RandomColorSystem = __COLOR_SYSTEM_TYPE(r, g, b, mode, &EnableGrayscale);
	}
}