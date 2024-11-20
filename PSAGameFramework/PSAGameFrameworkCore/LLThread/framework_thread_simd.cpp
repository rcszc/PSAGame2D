// framework_thread_simd.
#include "framework_thread.hpp"

using namespace std;
using namespace PSAG_LOGGER;

namespace PSAG_THREAD_SIMD {
#ifdef ENABLE_AVX256_CALC_FUNC

	PsagFP32SIMD256 operator+(const PsagFP32SIMD256& v, const PsagFP32SIMD256& s) { return _mm256_add_ps(v, s); }
	PsagFP32SIMD256 operator-(const PsagFP32SIMD256& v, const PsagFP32SIMD256& s) { return _mm256_sub_ps(v, s); }
	PsagFP32SIMD256 operator*(const PsagFP32SIMD256& v, const PsagFP32SIMD256& s) { return _mm256_mul_ps(v, s); }
	PsagFP32SIMD256 operator/(const PsagFP32SIMD256& v, const PsagFP32SIMD256& s) { return _mm256_div_ps(v, s); }

	PsagFP32SIMD256& operator+=(PsagFP32SIMD256& a, PsagFP32SIMD256 b) { a = a + b; return a; }
	PsagFP32SIMD256& operator-=(PsagFP32SIMD256& a, PsagFP32SIMD256 b) { a = a - b; return a; }
	PsagFP32SIMD256& operator*=(PsagFP32SIMD256& a, PsagFP32SIMD256 b) { a = a * b; return a; }
	PsagFP32SIMD256& operator/=(PsagFP32SIMD256& a, PsagFP32SIMD256 b) { a = a / b; return a; }

	PsagFP32SIMD256 PsagSimd8Fp32Load(float* dataptr, size_t offset, size_t bytes) {
#ifdef ENABLE_SIMD_8FP32_CHECK
		if (bytes != AVX256_32B_COUNT * sizeof(float)) {
			cerr << "SIMD_PTR_ERR! load-8-fp32" << endl;
			return PsagFp32Simd256b{};
		}
#endif
		float* Fp32Pointer = dataptr + offset;
		return _mm256_loadu_ps(Fp32Pointer);
	}

	PsagFP32SIMD256 PsagSimd8Fp32FillLoad(float* dataptr, size_t count_32b) {
		float VecTemp[AVX256_32B_COUNT] = { NULL };
		for (size_t i = 0; i < count_32b; ++i)
			VecTemp[i] = dataptr[i];
		return _mm256_loadu_ps(VecTemp);
	}

	void PsagSimd8Fp32Store(const PsagFP32SIMD256& vec, float* dataptr, size_t offset, size_t bytes) {
#ifdef ENABLE_SIMD_8FP32_CHECK
		if (bytes != AVX256_32B_COUNT * sizeof(float)) {
			cerr << "SIMD_PTR_ERR! store-8-fp32" << endl;
			return;
		}
#endif
		_mm256_storeu_ps(dataptr + offset, vec);
	}

	PsagFP32SIMD256 PsagSimdCalcFMADD(PsagFP32SIMD256 A, PsagFP32SIMD256 B, PsagFP32SIMD256 C) {
		// avx-isa: fused multiply-add, calc_time: 1-cycles.
		return _mm256_fmadd_ps(A, B, C);
	}

	PsagFP32SIMD256 PsagSimdCalcFABS(PsagFP32SIMD256 A) {
		// avx-isa: and_not(-0.0f), ieee754.
		return _mm256_andnot_ps(_mm256_set1_ps(-0.0f), A);
	}
	PsagFP32SIMD256 PsagSimdCalcSQRT(PsagFP32SIMD256 A) { return _mm256_sqrt_ps(A); }
	PsagFP32SIMD256 PsagSimdCalcLIMIT(PsagFP32SIMD256 A, PsagFP32SIMD256 B, LIMIT_CALC_TYPE MODE) {
		switch (MODE) {
		case(ModeMax): { return _mm256_max_ps(A, B); }
		case(ModeMin): { return _mm256_min_ps(A, B); }
		default: return PsagFP32SIMD256{};
		}
	}
	
	// update function. RCSZ [20240824]

	PsagFP32SIMD256 PsagSimdCalcRSQRT(PsagFP32SIMD256 A) {
		// avx-isa: reciprocal sqrt, faster but less precise than _mm256_sqrt_ps.
		return _mm256_rsqrt_ps(A);
	}

	PsagFP32SIMD256 PsagSimdCalcRECI(PsagFP32SIMD256 A) {
		// avx-isa: approximate reciprocal.
		return _mm256_rcp_ps(A);
	}

	PsagFP32SIMD256 PsagSimdCalcNEG(PsagFP32SIMD256 A) {
		// avx-isa: negate by XOR with sign bit.
		return _mm256_xor_ps(A, _mm256_set1_ps(-0.0f));
	}

	PsagFP32SIMD256 PsagSimdCalcHADD(PsagFP32SIMD256 A, PsagFP32SIMD256 B) {
		// avx-isa: horizontal add.
		return _mm256_hadd_ps(A, B);
	}

	PsagFP32SIMD256 PsagSimdCalcDOT(PsagFP32SIMD256 A, PsagFP32SIMD256 B) {
		// avx-isa: dot product by multiplying and horizontal add.
		PsagFP32SIMD256 MUL = _mm256_mul_ps(A, B);
		return _mm256_hadd_ps(MUL, MUL);
	}

#define M256_S1_PSVALUE _mm256_set1_ps(-1.0f)
	// avx-isa: compare equal.
	bool operator<(const PsagFP32SIMD256& v, const PsagFP32SIMD256& s) {
		return (bool)_mm256_testc_ps(_mm256_cmp_ps(v, s, _CMP_LT_OQ), M256_S1_PSVALUE);
	}
	// avx-isa: compare less than.
	bool operator>(const PsagFP32SIMD256& v, const PsagFP32SIMD256& s) {
		return (bool)_mm256_testc_ps(_mm256_cmp_ps(v, s, _CMP_GT_OQ), M256_S1_PSVALUE);
	}
	// avx-isa: compare less than or equal.
	bool operator<=(const PsagFP32SIMD256& v, const PsagFP32SIMD256& s) {
		return (bool)_mm256_testc_ps(_mm256_cmp_ps(v, s, _CMP_LE_OQ), M256_S1_PSVALUE);
	}
	// avx-isa: compare greater than.
	bool operator>=(const PsagFP32SIMD256& v, const PsagFP32SIMD256& s) {
		return (bool)_mm256_testc_ps(_mm256_cmp_ps(v, s, _CMP_GE_OQ), M256_S1_PSVALUE);
	}
	// avx-isa: compare greater than or equal.
	bool operator==(const PsagFP32SIMD256& v, const PsagFP32SIMD256& s) {
		return (bool)_mm256_testc_ps(_mm256_cmp_ps(v, s, _CMP_EQ_OQ), M256_S1_PSVALUE);
	}
#endif
}