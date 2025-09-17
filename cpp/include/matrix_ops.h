/**
 * UMICP Matrix Operations Header
 * High-performance vector and matrix operations for AI model communication
 */

#ifndef UMICP_MATRIX_OPS_H
#define UMICP_MATRIX_OPS_H

#include "umicp_types.h"

// SIMD headers
#if defined(__AVX512F__)
#include <immintrin.h>
#elif defined(__AVX2__)
#include <immintrin.h>
#elif defined(__SSE__)
#include <xmmintrin.h>
#endif

namespace umicp {

// Matrix operations class with SIMD optimizations
class MatrixOps {
public:
    // Matrix addition: result = a + b
    static Result<void> add(const float* a, const float* b, float* result, size_t rows, size_t cols);

    // Matrix multiplication: result = a * b (m x n) * (n x p) = (m x p)
    static Result<void> multiply(const float* a, const float* b, float* result,
                                size_t m, size_t n, size_t p);

    // Matrix transpose: result = a^T
    static Result<void> transpose(const float* input, float* output, size_t rows, size_t cols);

    // Dot product of two vectors
    static Result<void> dot_product(const float* a, const float* b, float* result, size_t size);

    // Vector/matrix normalization (L2 normalization)
    static Result<void> normalize(float* matrix, size_t rows, size_t cols);

    // Cosine similarity between two vectors
    static Result<void> cosine_similarity(const float* a, const float* b, float* result, size_t size);

private:
    // Helper functions for SIMD operations
#ifdef __AVX2__
    static inline float horizontal_sum_avx(__m256 v) {
        __m128 hi = _mm256_extractf128_ps(v, 1);
        __m128 lo = _mm256_extractf128_ps(v, 0);
        __m128 sum = _mm_add_ps(hi, lo);
        sum = _mm_add_ps(sum, _mm_movehl_ps(sum, sum));
        sum = _mm_add_ss(sum, _mm_shuffle_ps(sum, sum, 0x55));
        return _mm_cvtss_f32(sum);
    }
#endif

#ifdef __AVX512F__
    static inline float horizontal_sum_avx512(__m512 v) {
        return _mm512_reduce_add_ps(v);
    }
#endif
};

} // namespace umicp

#endif // UMICP_MATRIX_OPS_H
