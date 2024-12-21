// framework_thread_simd.
#include "framework_thread_simd.hpp"

using namespace std;

namespace PSAG_THREAD_SIMD {
#ifdef ENABLE_AVX256_CALC_FUNC

	SML256FP32 SML_FuncSimd_8FP32LOAD(const float* dataptr, size_t offset) {
		const float* Fp32Pointer = dataptr + offset;
		return _mm256_loadu_ps(Fp32Pointer);
	}

	void SML_FuncSimd_8FP32STORE(SML256FP32 vec, float* dataptr, size_t offset) {
		_mm256_storeu_ps(dataptr + offset, vec);
	}

	SML256FP32 SML_FuncSimd_8FP32FILL(const float* dataptr, size_t count_32b) {
		float VecTemp[AVX256_FP32BIT_COUNT] = { NULL };
		memcpy(VecTemp, dataptr, count_32b * sizeof(float));
		return _mm256_loadu_ps(VecTemp);
	}

	void SML_FuncSimdUpdateReg(const float* vec_data, RegisterMask mask, SML256FP32* vec_register) {
		__m256i IMM256MASK = _mm256_set_epi32(
			((mask >> 7) & 1) ? -1 : 0, ((mask >> 6) & 1) ? -1 : 0,
			((mask >> 5) & 1) ? -1 : 0, ((mask >> 4) & 1) ? -1 : 0,
			((mask >> 3) & 1) ? -1 : 0, ((mask >> 2) & 1) ? -1 : 0,
			((mask >> 1) & 1) ? -1 : 0, ((mask >> 0) & 1) ? -1 : 0
		);
		SML256FP32 DataVecNew  = _mm256_maskload_ps(vec_data, IMM256MASK);
		SML256FP32 DataVecMask = _mm256_castsi256_ps(IMM256MASK);

		*vec_register = _mm256_blendv_ps(*vec_register, DataVecMask, DataVecNew);
	}

	SML256FP32 SML_SIMD_CALC_FMADD(SML256FP32 A, SML256FP32 B, SML256FP32 C) {
		// avx-isa: fused multiply-add, calc_time: 1-cycles.
		return _mm256_fmadd_ps(A, B, C);
	}

	SML256FP32 SML_SIMD_CALC_FABS(SML256FP32 A) {
		// avx-isa: and_not(-0.0f), ieee754.
		return _mm256_andnot_ps(_mm256_set1_ps(-0.0f), A);
	}

	SML256FP32 SML_SIMD_CALC_LIMIT(SML256FP32 A, SML256FP32 B, LIMIT_CALC_TYPE MODE) {
		switch (MODE) {
		case(ModeMax): { return _mm256_max_ps(A, B); }
		case(ModeMin): { return _mm256_min_ps(A, B); }
		default: return SML256FP32{};
		}
	}
	
	// update function. rcsz 20240824.
	// simd api ”… chat_gpt Ã·π©. 

	SML256FP32 SML_SIMD_CALC_RECI(SML256FP32 A) {
		// avx-isa: approximate reciprocal.
		return _mm256_rcp_ps(A);
	}

	SML256FP32 SML_SIMD_CALC_NEG(SML256FP32 A) {
		// avx-isa: negate by XOR with sign bit.
		return _mm256_xor_ps(A, _mm256_set1_ps(-0.0f));
	}

	SML256FP32 SML_SIMD_CALC_HADD(SML256FP32 A, SML256FP32 B) {
		// avx-isa: horizontal add.
		return _mm256_hadd_ps(A, B);
	}

	SML256FP32 SML_SIMD_CALC_DOT(SML256FP32 A, SML256FP32 B) {
		// avx-isa: dot product by multiplying and horizontal add.
		SML256FP32 MUL = _mm256_mul_ps(A, B);
		return _mm256_hadd_ps(MUL, MUL);
	}

	SML256FP32 SML_SIMD_CALC_RSQRT(SML256FP32 A) { return _mm256_rsqrt_ps(A); }
	SML256FP32 SML_SIMD_CALC_SQRT (SML256FP32 A) { return _mm256_sqrt_ps(A);  }
#endif
}