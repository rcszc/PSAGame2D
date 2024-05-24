// framework_thread_simd.
#include "framework_thread.hpp"

using namespace std;
using namespace PSAG_LOGGER;

namespace PSAG_THREAD_SIMD {
#ifdef ENABLE_AVX256_CALC_FUNC

	PsagFp32Simd256b PsagSimdLoad8Fp32(float* dataptr, size_t offset, size_t bytes) {
#ifdef ENABLE_SIMD_LOAD8FP32_TEST
		if (bytes != 32) return PsagFp32Simd256b{};
#endif
		float* Fp32Pointer = dataptr + offset;
		return _mm256_loadu_ps(Fp32Pointer);
	}

	PsagFp32Simd256b PsagSimdLoad8Fp32Fill(float* dataptr, size_t count_32b) {
		float VecTemp[AVX256_32B_COUNT] = { NULL };
		for (size_t i = 0; i < count_32b; ++i)
			VecTemp[i] = dataptr[i];
		return _mm256_loadu_ps(VecTemp);
	}

	PsagFp32Simd256b PsagSimdCalcADD(PsagFp32Simd256b A, PsagFp32Simd256b B) { return _mm256_add_ps(A, B); }
	PsagFp32Simd256b PsagSimdCalcSUB(PsagFp32Simd256b A, PsagFp32Simd256b B) { return _mm256_sub_ps(A, B); }
	PsagFp32Simd256b PsagSimdCalcMUL(PsagFp32Simd256b A, PsagFp32Simd256b B) { return _mm256_mul_ps(A, B); }
	PsagFp32Simd256b PsagSimdCalcDIV(PsagFp32Simd256b A, PsagFp32Simd256b B) { return _mm256_div_ps(A, B); }

	PsagFp32Simd256b PsagSimdCalcFMADD(PsagFp32Simd256b A, PsagFp32Simd256b B, PsagFp32Simd256b C) {
		// avx-isa: fused multiply-add, calc_time: 1-cycles.
		return _mm256_fmadd_ps(A, B, C);
	}

	PsagFp32Simd256b PsagSimdCalcFABS(PsagFp32Simd256b A) {
		// avx-isa: and_not(-0.0f), ieee754.
		return _mm256_andnot_ps(_mm256_set1_ps(-0.0f), A);
	}
	PsagFp32Simd256b PsagSimdCalcSQRT(PsagFp32Simd256b A) { return _mm256_sqrt_ps(A); }
	PsagFp32Simd256b PsagSimdCalcLIMIT(PsagFp32Simd256b A, PsagFp32Simd256b B, LIMIT_CALC_TYPE MODE) {
		switch (MODE) {
		case(ModeMax): { return _mm256_max_ps(A, B); }
		case(ModeMin): { return _mm256_min_ps(A, B); }
		}
	}
#endif
}