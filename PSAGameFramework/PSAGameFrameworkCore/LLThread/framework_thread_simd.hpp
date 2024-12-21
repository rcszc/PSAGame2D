// framework_thread_simd. TEST-MODULE.
// 实验性模块: version-20241123. 目前只兼容 INTEL AVX 指令集架构.
// 实验性移植: SML, SMLC 内部实验性项目.
// INTEL[x] "immintrin.h", AMD[ ] "immintrin.h"

#ifndef _FRAMEWORK_THREAD_SIMD_HPP
#define _FRAMEWORK_THREAD_SIMD_HPP
// LLThread PSA-L.5 (=>logger).
#include "../LLLogger/framework_logger.hpp"
// device intel-avx-isa.
#include <immintrin.h>
#define AVX256_FP32BIT_COUNT 8

#define ENABLE_AVX256_CALC_FUNC
// imm256 register 256-bit float32 * 8, 32-bytes.
using SML256FP32 = __m256;
// PSAG thread(calc) function: SIMD AVX-256, X86-64.
namespace PSAG_THREAD_SIMD {
#ifdef ENABLE_AVX256_CALC_FUNC

    SML256FP32 SML_FuncSimd_8FP32LOAD(const float* dataptr, size_t offset);
    void       SML_FuncSimd_8FP32STORE(SML256FP32 vec, float* dataptr, size_t offset);
    SML256FP32 SML_FuncSimd_8FP32FILL(const float* dataptr, size_t count_32b);

    inline SML256FP32 operator+(const SML256FP32& v, const SML256FP32& s) { return _mm256_add_ps(v, s); }
    inline SML256FP32 operator-(const SML256FP32& v, const SML256FP32& s) { return _mm256_sub_ps(v, s); }
    inline SML256FP32 operator*(const SML256FP32& v, const SML256FP32& s) { return _mm256_mul_ps(v, s); }
    inline SML256FP32 operator/(const SML256FP32& v, const SML256FP32& s) { return _mm256_div_ps(v, s); }

    inline SML256FP32 operator+(const SML256FP32& v, const float& s) { return v + _mm256_set1_ps(s); }
    inline SML256FP32 operator-(const SML256FP32& v, const float& s) { return v - _mm256_set1_ps(s); }
    inline SML256FP32 operator*(const SML256FP32& v, const float& s) { return v * _mm256_set1_ps(s); }
    inline SML256FP32 operator/(const SML256FP32& v, const float& s) { return v / _mm256_set1_ps(s); }

    inline SML256FP32& operator+=(SML256FP32& a, const float& b) { a = a + b; return a; }
    inline SML256FP32& operator-=(SML256FP32& a, const float& b) { a = a - b; return a; }
    inline SML256FP32& operator*=(SML256FP32& a, const float& b) { a = a * b; return a; }
    inline SML256FP32& operator/=(SML256FP32& a, const float& b) { a = a / b; return a; }

    inline SML256FP32& operator+=(SML256FP32& a, SML256FP32 b) { a = a + b; return a; }
    inline SML256FP32& operator-=(SML256FP32& a, SML256FP32 b) { a = a - b; return a; }
    inline SML256FP32& operator*=(SML256FP32& a, SML256FP32 b) { a = a * b; return a; }
    inline SML256FP32& operator/=(SML256FP32& a, SML256FP32 b) { a = a / b; return a; }

    using RegisterMask = uint8_t;
    void SML_FuncSimdUpdateReg(const float* vec_data, RegisterMask mask, SML256FP32* vec_register);

    class PSAG_SIMD_LOAD {
    protected:
        std::vector<float>* DatasetPointer = nullptr;
    public:
        PSAG_SIMD_LOAD(std::vector<float>* ref) : DatasetPointer(ref) {};
        size_t GET_SIMD256_ITEMS_NUMBER() {
            return DatasetPointer->size() / AVX256_FP32BIT_COUNT;
        }
        SML256FP32 LOAD_SIMD256_ITEM(size_t index) {
            return SML_FuncSimd_8FP32LOAD(DatasetPointer->data(), index * AVX256_FP32BIT_COUNT);
        }
    };

    class PSAG_SIMD_STORE {
    protected:
        std::vector<float>* DatasetPointer = nullptr;
    public:
        PSAG_SIMD_STORE(std::vector<float>* ref) : DatasetPointer(ref) {};
        size_t GET_SIMD256_ITEMS_NUMBER() {
            return DatasetPointer->size() / AVX256_FP32BIT_COUNT;
        }
        void STORE_SIMD256_ITEM(const SML256FP32& value, size_t index) {
            SML_FuncSimd_8FP32STORE(value, DatasetPointer->data(), index * AVX256_FP32BIT_COUNT);
        }
    };

    SML256FP32 operator+(const SML256FP32& v, const SML256FP32& s);
    SML256FP32 operator-(const SML256FP32& v, const SML256FP32& s);
    SML256FP32 operator*(const SML256FP32& v, const SML256FP32& s);
    SML256FP32 operator/(const SML256FP32& v, const SML256FP32& s);

    SML256FP32& operator+=(SML256FP32& a, SML256FP32 b);
    SML256FP32& operator-=(SML256FP32& a, SML256FP32 b);
    SML256FP32& operator*=(SML256FP32& a, SML256FP32 b);
    SML256FP32& operator/=(SML256FP32& a, SML256FP32 b);

    // fma(fused multiply-add), res = a * b + c, [matrix calc].
    SML256FP32 SML_SIMD_CALC_FMADD(SML256FP32 A, SML256FP32 B, SML256FP32 C);

    SML256FP32 SML_SIMD_CALC_FABS(SML256FP32 A);
    enum LIMIT_CALC_TYPE {
        ModeMax = 1 << 1,
        ModeMin = 1 << 2
    };
    SML256FP32 SML_SIMD_CALC_LIMIT(SML256FP32 A, SML256FP32 B, LIMIT_CALC_TYPE MODE);

    template <int mask>
    // avx-isa: blend values based mask [0,255].
    SML256FP32 SML_SIMD_CALC_BLEND(SML256FP32 A, SML256FP32 B) {
        return _mm256_blend_ps(A, B, mask);
    }
    
    SML256FP32 SML_SIMD_CALC_RECI (SML256FP32 A);
    SML256FP32 SML_SIMD_CALC_NEG  (SML256FP32 A);
    SML256FP32 SML_SIMD_CALC_HADD (SML256FP32 A, SML256FP32 B);
    SML256FP32 SML_SIMD_CALC_DOT  (SML256FP32 A, SML256FP32 B);
    SML256FP32 SML_SIMD_CALC_SQRT (SML256FP32 A);
    SML256FP32 SML_SIMD_CALC_RSQRT(SML256FP32 A);

#define M256_SETPS_VALUE _mm256_set1_ps(-1.0f)
    // avx-isa: compare equal.
    inline bool operator<(const SML256FP32& v, const SML256FP32& s) {
        return (bool)_mm256_testc_ps(_mm256_cmp_ps(v, s, _CMP_LT_OQ), M256_SETPS_VALUE);
    }
    // avx-isa: compare less than.
    inline bool operator>(const SML256FP32& v, const SML256FP32& s) {
        return (bool)_mm256_testc_ps(_mm256_cmp_ps(v, s, _CMP_GT_OQ), M256_SETPS_VALUE);
    }
    // avx-isa: compare less than or equal.
    inline bool operator<=(const SML256FP32& v, const SML256FP32& s) {
        return (bool)_mm256_testc_ps(_mm256_cmp_ps(v, s, _CMP_LE_OQ), M256_SETPS_VALUE);
    }
    // avx-isa: compare greater than.
    inline bool operator>=(const SML256FP32& v, const SML256FP32& s) {
        return (bool)_mm256_testc_ps(_mm256_cmp_ps(v, s, _CMP_GE_OQ), M256_SETPS_VALUE);
    }
    // avx-isa: compare greater than or equal.
    inline bool operator==(const SML256FP32& v, const SML256FP32& s) {
        return (bool)_mm256_testc_ps(_mm256_cmp_ps(v, s, _CMP_EQ_OQ), M256_SETPS_VALUE);
    }
#endif
}

#endif