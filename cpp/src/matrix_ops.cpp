/**
 * UMICP Matrix Operations
 * High-performance vector and matrix operations for AI model communication
 */

#include "umicp_types.h"
#include "matrix_ops.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <immintrin.h>  // SIMD instructions

namespace umicp {

// SIMD-optimized matrix addition
Result<void> MatrixOps::add(const float* a, const float* b, float* result, size_t rows, size_t cols) {
    const size_t total_elements = rows * cols;

    if (!a || !b || !result) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Null pointer argument");
    }

    if (rows == 0 || cols == 0) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Matrix dimensions must be greater than 0");
    }

    // For size mismatch test, we need to validate that the arrays are large enough
    // But we can't determine the actual array size from pointers, so we'll skip this check
    // and rely on the caller to provide appropriately sized arrays

#ifdef __AVX512F__
    // AVX-512 optimized version
    const size_t vector_size = 16; // 512 bits / 32 bits per float
    const size_t vectorized_end = (total_elements / vector_size) * vector_size;

    for (size_t i = 0; i < vectorized_end; i += vector_size) {
        __m512 va = _mm512_loadu_ps(&a[i]);
        __m512 vb = _mm512_loadu_ps(&b[i]);
        __m512 vr = _mm512_add_ps(va, vb);
        _mm512_storeu_ps(&result[i], vr);
    }

    // Handle remaining elements
    for (size_t i = vectorized_end; i < total_elements; ++i) {
        result[i] = a[i] + b[i];
    }
#elif defined(__AVX2__)
    // AVX2 optimized version
    const size_t vector_size = 8; // 256 bits / 32 bits per float
    const size_t vectorized_end = (total_elements / vector_size) * vector_size;

    for (size_t i = 0; i < vectorized_end; i += vector_size) {
        __m256 va = _mm256_loadu_ps(&a[i]);
        __m256 vb = _mm256_loadu_ps(&b[i]);
        __m256 vr = _mm256_add_ps(va, vb);
        _mm256_storeu_ps(&result[i], vr);
    }

    // Handle remaining elements
    for (size_t i = vectorized_end; i < total_elements; ++i) {
        result[i] = a[i] + b[i];
    }
#else
    // Fallback scalar version
    for (size_t i = 0; i < total_elements; ++i) {
        result[i] = a[i] + b[i];
    }
#endif

    return Result<void>();
}

// SIMD-optimized matrix multiplication (naive implementation for MVP)
Result<void> MatrixOps::multiply(const float* a, const float* b, float* result,
                                size_t m, size_t n, size_t p) {
    if (!a || !b || !result) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Null pointer argument");
    }

    if (m == 0 || n == 0 || p == 0) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Matrix dimensions must be greater than 0");
    }

    // Initialize result matrix to zero
    std::fill(result, result + m * p, 0.0f);

#ifdef __AVX512F__
    // AVX-512 optimized matrix multiplication
    const size_t block_size = 16;

    for (size_t i = 0; i < m; ++i) {
        for (size_t j = 0; j < p; ++j) {
            __m512 sum = _mm512_setzero_ps();

            for (size_t k = 0; k < n; k += block_size) {
                const size_t remaining = std::min(block_size, n - k);

                // Load blocks
                __m512 va, vb;
                if (remaining == block_size) {
                    va = _mm512_loadu_ps(&a[i * n + k]);
                    vb = _mm512_loadu_ps(&b[k * p + j]);
                } else {
                    // Handle partial loads
                    float temp_a[16] = {0};
                    float temp_b[16] = {0};
                    std::memcpy(temp_a, &a[i * n + k], remaining * sizeof(float));
                    for (size_t r = 0; r < remaining; ++r) {
                        temp_b[r] = b[(k + r) * p + j];
                    }
                    va = _mm512_loadu_ps(temp_a);
                    vb = _mm512_loadu_ps(temp_b);
                }

                sum = _mm512_fmadd_ps(va, vb, sum);
            }

            // Horizontal sum and store
            result[i * p + j] = _mm512_reduce_add_ps(sum);
        }
    }
#else
    // Fallback scalar matrix multiplication
    for (size_t i = 0; i < m; ++i) {
        for (size_t j = 0; j < p; ++j) {
            for (size_t k = 0; k < n; ++k) {
                result[i * p + j] += a[i * n + k] * b[k * p + j];
            }
        }
    }
#endif

    return Result<void>();
}

// SIMD-optimized matrix transpose
Result<void> MatrixOps::transpose(const float* input, float* output, size_t rows, size_t cols) {
    if (!input || !output) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Null pointer argument");
    }

#ifdef __AVX512F__
    // AVX-512 optimized transpose
    const size_t block_size = 16;

    for (size_t i = 0; i < rows; i += block_size) {
        for (size_t j = 0; j < cols; j += block_size) {
            const size_t i_end = std::min(i + block_size, rows);
            const size_t j_end = std::min(j + block_size, cols);

            // Transpose block
            for (size_t bi = i; bi < i_end; ++bi) {
                for (size_t bj = j; bj < j_end; ++bj) {
                    output[bj * rows + bi] = input[bi * cols + bj];
                }
            }
        }
    }
#elif defined(__AVX2__)
    // AVX2 optimized transpose using shuffle operations
    const size_t vector_size = 8;

    for (size_t i = 0; i < rows; i += vector_size) {
        for (size_t j = 0; j < cols; j += vector_size) {
            // Load 8x8 block
            __m256 row0 = _mm256_loadu_ps(&input[(i + 0) * cols + j]);
            __m256 row1 = _mm256_loadu_ps(&input[(i + 1) * cols + j]);
            __m256 row2 = _mm256_loadu_ps(&input[(i + 2) * cols + j]);
            __m256 row3 = _mm256_loadu_ps(&input[(i + 3) * cols + j]);
            __m256 row4 = _mm256_loadu_ps(&input[(i + 4) * cols + j]);
            __m256 row5 = _mm256_loadu_ps(&input[(i + 5) * cols + j]);
            __m256 row6 = _mm256_loadu_ps(&input[(i + 6) * cols + j]);
            __m256 row7 = _mm256_loadu_ps(&input[(i + 7) * cols + j]);

            // Transpose 8x8 block using shuffle
            __m256 tmp0 = _mm256_unpacklo_ps(row0, row1);
            __m256 tmp1 = _mm256_unpackhi_ps(row0, row1);
            __m256 tmp2 = _mm256_unpacklo_ps(row2, row3);
            __m256 tmp3 = _mm256_unpackhi_ps(row2, row3);
            __m256 tmp4 = _mm256_unpacklo_ps(row4, row5);
            __m256 tmp5 = _mm256_unpackhi_ps(row4, row5);
            __m256 tmp6 = _mm256_unpacklo_ps(row6, row7);
            __m256 tmp7 = _mm256_unpackhi_ps(row6, row7);

            // Continue with more shuffles for full transpose...

            // For now, fallback to scalar for remaining elements
            const size_t i_end = std::min(i + vector_size, rows);
            const size_t j_end = std::min(j + vector_size, cols);

            for (size_t bi = i; bi < i_end; ++bi) {
                for (size_t bj = j; bj < j_end; ++bj) {
                    output[bj * rows + bi] = input[bi * cols + bj];
                }
            }
        }
    }
#else
    // Fallback scalar transpose
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            output[j * rows + i] = input[i * cols + j];
        }
    }
#endif

    return Result<void>();
}

// SIMD-optimized dot product
Result<void> MatrixOps::dot_product(const float* a, const float* b, float* result, size_t size) {
    if (!a || !b || !result) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Null pointer argument");
    }

    *result = 0.0f;

#ifdef __AVX512F__
    // AVX-512 optimized dot product
    __m512 sum = _mm512_setzero_ps();
    const size_t vector_size = 16;
    const size_t vectorized_end = (size / vector_size) * vector_size;

    for (size_t i = 0; i < vectorized_end; i += vector_size) {
        __m512 va = _mm512_loadu_ps(&a[i]);
        __m512 vb = _mm512_loadu_ps(&b[i]);
        sum = _mm512_fmadd_ps(va, vb, sum);
    }

    *result = _mm512_reduce_add_ps(sum);

    // Handle remaining elements
    for (size_t i = vectorized_end; i < size; ++i) {
        *result += a[i] * b[i];
    }
#elif defined(__AVX2__)
    // AVX2 optimized dot product
    __m256 sum = _mm256_setzero_ps();
    const size_t vector_size = 8;
    const size_t vectorized_end = (size / vector_size) * vector_size;

    for (size_t i = 0; i < vectorized_end; i += vector_size) {
        __m256 va = _mm256_loadu_ps(&a[i]);
        __m256 vb = _mm256_loadu_ps(&b[i]);
        sum = _mm256_fmadd_ps(va, vb, sum);
    }

    // Horizontal sum
    __m128 sum128 = _mm_add_ps(_mm256_extractf128_ps(sum, 0), _mm256_extractf128_ps(sum, 1));
    sum128 = _mm_add_ps(sum128, _mm_movehl_ps(sum128, sum128));
    sum128 = _mm_add_ss(sum128, _mm_shuffle_ps(sum128, sum128, 0x55));
    _mm_store_ss(result, sum128);

    // Handle remaining elements
    for (size_t i = vectorized_end; i < size; ++i) {
        *result += a[i] * b[i];
    }
#else
    // Fallback scalar dot product
    for (size_t i = 0; i < size; ++i) {
        *result += a[i] * b[i];
    }
#endif

    return Result<void>();
}

// Vector normalization
Result<void> MatrixOps::normalize(float* matrix, size_t rows, size_t cols) {
    if (!matrix) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Null pointer argument");
    }

    // const size_t total_elements = rows * cols; // TODO: Use this for validation

    // Normalize each row (assuming row-major order for embeddings)
    for (size_t i = 0; i < rows; ++i) {
        const size_t row_start = i * cols;

        // Calculate L2 norm
        float norm = 0.0f;
#ifdef __AVX512F__
        __m512 sum = _mm512_setzero_ps();
        const size_t vector_size = 16;
        const size_t vectorized_end = (cols / vector_size) * vector_size;

        for (size_t j = 0; j < vectorized_end; j += vector_size) {
            __m512 val = _mm512_loadu_ps(&matrix[row_start + j]);
            sum = _mm512_fmadd_ps(val, val, sum);
        }

        norm = _mm512_reduce_add_ps(sum);

        // Handle remaining elements
        for (size_t j = vectorized_end; j < cols; ++j) {
            norm += matrix[row_start + j] * matrix[row_start + j];
        }
#else
        for (size_t j = 0; j < cols; ++j) {
            norm += matrix[row_start + j] * matrix[row_start + j];
        }
#endif

        norm = std::sqrt(norm);

        if (norm > 0.0f) {
            // Normalize the vector
            const float inv_norm = 1.0f / norm;
#ifdef __AVX512F__
            __m512 inv_norm_vec = _mm512_set1_ps(inv_norm);
            for (size_t j = 0; j < vectorized_end; j += vector_size) {
                __m512 val = _mm512_loadu_ps(&matrix[row_start + j]);
                val = _mm512_mul_ps(val, inv_norm_vec);
                _mm512_storeu_ps(&matrix[row_start + j], val);
            }

            // Handle remaining elements
            for (size_t j = vectorized_end; j < cols; ++j) {
                matrix[row_start + j] *= inv_norm;
            }
#else
            for (size_t j = 0; j < cols; ++j) {
                matrix[row_start + j] *= inv_norm;
            }
#endif
        }
    }

    return Result<void>();
}

// Cosine similarity between two vectors
Result<void> MatrixOps::cosine_similarity(const float* a, const float* b, float* result, size_t size) {
    if (!a || !b || !result) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Null pointer argument");
    }

    // Calculate dot product
    float dot_prod = 0.0f;
    auto dot_result = dot_product(a, b, &dot_prod, size);
    if (!dot_result.is_success()) {
        return dot_result;
    }

    // Calculate magnitudes
    float mag_a = 0.0f, mag_b = 0.0f;
    auto mag_a_result = dot_product(a, a, &mag_a, size);
    auto mag_b_result = dot_product(b, b, &mag_b, size);

    if (!mag_a_result.is_success() || !mag_b_result.is_success()) {
        return Result<void>(ErrorCode::INVALID_ARGUMENT, "Failed to calculate magnitudes");
    }

    mag_a = std::sqrt(mag_a);
    mag_b = std::sqrt(mag_b);

    if (mag_a == 0.0f || mag_b == 0.0f) {
        *result = 0.0f; // Handle zero vectors
    } else {
        *result = dot_prod / (mag_a * mag_b);
    }

    return Result<void>();
}

} // namespace umicp
