// framework_def_math. RCSZ.

#ifndef _FRAMEWORK_DEF_MATH_HPP
#define _FRAMEWORK_DEF_MATH_HPP

// PSA-V0.1.2 RENDER-COORD ±ê×¼. [RENDER_COORD_01]
// framework const ortho space_coord.
constexpr float SystemRenderingOrthoSpace = 100.0f;

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

struct PsagMatrix2 { float matrix[2 * 2]; };
struct PsagMatrix3 { float matrix[3 * 3]; };
struct PsagMatrix4 { float matrix[4 * 4]; };

#endif