// framework_thread_simd.
#include "framework_thread.hpp"

using namespace std;
using namespace PSAG_LOGGER;

namespace PSAG_THREAD_SIMD {
#ifdef ENABLE_AVX256_CALC_FUNC

	PsagFp32Simd256b PsagSimdLoad8Fp32(float* dataptr, size_t offset, size_t bytes) {
#ifdef ENABLE_SIMD_LOAD8FP32_TEST
		if (bytes != 32) {
			cerr << "SIMD_PTR_ERR! (Load8Fp32)" << endl;
			return PsagFp32Simd256b{};
		}
#endif
		float* Fp32Pointer = dataptr + offset;
		return _mm256_loadu_ps(Fp32Pointer);
	}

	void PsagSimdStore8Fp32(PsagFp32Simd256b vec, float* dataptr, size_t bytes) {
#ifdef ENABLE_SIMD_LOAD8FP32_TEST
		if (bytes != 32) {
			cerr << "SIMD_PTR_ERR! (Store8Fp32)" << endl;
			return;
		}
#endif
		_mm256_storeu_ps(dataptr, vec);
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
		default: return PsagFp32Simd256b{};
		}
	}
	
	// update function. RCSZ [20240824]

	PsagFp32Simd256b PsagSimdCalcRSQRT(PsagFp32Simd256b A) {
		// avx-isa: reciprocal sqrt, faster but less precise than _mm256_sqrt_ps.
		return _mm256_rsqrt_ps(A);
	}

	PsagFp32Simd256b PsagSimdCalcRECI(PsagFp32Simd256b A) {
		// avx-isa: approximate reciprocal.
		return _mm256_rcp_ps(A);
	}

	PsagFp32Simd256b PsagSimdCalcNEG(PsagFp32Simd256b A) {
		// avx-isa: negate by XOR with sign bit.
		return _mm256_xor_ps(A, _mm256_set1_ps(-0.0f));
	}

	PsagFp32Simd256b PsagSimdCalcHADD(PsagFp32Simd256b A, PsagFp32Simd256b B) {
		// avx-isa: horizontal add.
		return _mm256_hadd_ps(A, B);
	}

	PsagFp32Simd256b PsagSimdCalcDOT(PsagFp32Simd256b A, PsagFp32Simd256b B) {
		// avx-isa: dot product by multiplying and horizontal add.
		PsagFp32Simd256b MUL = _mm256_mul_ps(A, B);
		return _mm256_hadd_ps(MUL, MUL);
	}

#define M256_S1_PSVALUE _mm256_set1_ps(-1.0f)
	bool PsagSimdCompareEQ(PsagFp32Simd256b A, PsagFp32Simd256b B) {
		// avx-isa: compare equal.
		return _mm256_testc_ps(_mm256_cmp_ps(A, B, _CMP_EQ_OQ), M256_S1_PSVALUE);
	}

	bool PsagSimdCompareLT(PsagFp32Simd256b A, PsagFp32Simd256b B) {
		// avx-isa: compare less than.
		return _mm256_testc_ps(_mm256_cmp_ps(A, B, _CMP_LT_OQ), M256_S1_PSVALUE);
	}

	bool PsagSimdCompareLE(PsagFp32Simd256b A, PsagFp32Simd256b B) {
		// avx-isa: compare less than or equal.
		return _mm256_testc_ps(_mm256_cmp_ps(A, B, _CMP_LE_OQ), M256_S1_PSVALUE);
	}

	bool PsagSimdCompareGT(PsagFp32Simd256b A, PsagFp32Simd256b B) {
		// avx-isa: compare greater than.
		return _mm256_testc_ps(_mm256_cmp_ps(A, B, _CMP_GT_OQ), M256_S1_PSVALUE);
	}

	bool PsagSimdCompareGE(PsagFp32Simd256b A, PsagFp32Simd256b B) {
		// avx-isa: compare greater than or equal.
		return _mm256_testc_ps(_mm256_cmp_ps(A, B, _CMP_GE_OQ), M256_S1_PSVALUE);
	}
#endif
}