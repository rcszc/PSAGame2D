// framework_def_math. RCSZ.

#ifndef _FRAMEWORK_DEF_MATH_HPP
#define _FRAMEWORK_DEF_MATH_HPP
#define POMELO_STAR_GAME2D_DEF_MATHS

// PSA-V0.1.2 RENDER-COORD ±ê×¼. [RENDER_COORD_01]
// framework const ortho space_coord.
constexpr float SystemRenderingOrthoSpace = 100.0f;

#define PSAG_M_PI 3.14159265f
#define PSAG_M_DEGRAD(deg) ((deg) * (PSAG_M_PI / 180.0f))
#define PSAG_M_RADDEG(rad) ((rad) * (180.0f / PSAG_M_PI))

#define __psag_bit_min(a, b) ((b) ^ (((a) ^ (b)) & -((a) < (b))))
#define __psag_bit_max(a, b) ((a) ^ (((a) ^ (b)) & -((a) < (b))))

#define PSAG_MEM_FMT_STD140 alignas(16)

// vector 2d x, y.
template <typename mvec>
struct Vector2T {
	mvec vector_x, vector_y;
	constexpr Vector2T() : vector_x{}, vector_y{} {}
	constexpr Vector2T(mvec x, mvec y) : vector_x(x), vector_y(y) {}

	mvec* data() { return &vector_x; }
	const mvec* data() const { return &vector_x; }
};

// vector 3d x, y, z.
template <typename mvec>
struct Vector3T {
	mvec vector_x, vector_y, vector_z;
	constexpr Vector3T() : vector_x{}, vector_y{}, vector_z{} {}
	constexpr Vector3T(mvec x, mvec y, mvec z) : vector_x(x), vector_y(y), vector_z(z) {}

	mvec* data() { return &vector_x; }
	const mvec* data() const { return &vector_x; }
};

// vector 4d x, y, z, w.
template <typename mvec>
struct Vector4T {
	mvec vector_x, vector_y, vector_z, vector_w;
	constexpr Vector4T() : vector_x{}, vector_y{}, vector_z{}, vector_w{} {}
	constexpr Vector4T(mvec x, mvec y, mvec z, mvec w) : vector_x(x), vector_y(y), vector_z(z), vector_w(w) {}

	mvec* data() { return &vector_x; }
	const mvec* data() const { return &vector_x; }
};

struct PSAG_MEM_FMT_STD140 PsagMatrix2 { float matrix[2 * 2]; };
struct PSAG_MEM_FMT_STD140 PsagMatrix3 { float matrix[3 * 3]; };
struct PSAG_MEM_FMT_STD140 PsagMatrix4 { float matrix[4 * 4]; };

#define PsagClamp(val, min, max) ((val) < (min) ? (min) : ((val) > (max) ? (max) : (val)))

#endif