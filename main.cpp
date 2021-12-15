#include <iostream>

#include <oneapi/tbb/info.h>
#include <oneapi/tbb/parallel_for.h>
#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/task_arena.h>
#include <oneapi/tbb/tick_count.h>

#include <immintrin.h>

void simple_copy(float ** const src, float * const dst, const size_t n_rows, const size_t n_cols);
void parallel_copy(float ** const src, float * const dst, const size_t n_rows, const size_t n_cols);
void copy_block(void const * ptr_min, size_t const * offsets, void * dst, const size_t n_rows, const size_t n_cols);
void optimized_copy(float ** const src, float * const dst, const size_t n_rows, const size_t n_cols);

int main() {
    constexpr size_t n_rows = 4 * 1024 * 1024;
    constexpr size_t n_cols = 64;

    float ** const col_matrix = new float*[n_cols];
    for (size_t i = 0; i < n_cols; ++i)
        col_matrix[i] = new float[n_rows];

    float * const continuous_row_matrix = new float[n_rows * n_cols];

    constexpr size_t n_run = 10;
    oneapi::tbb::tick_count t0, t1;
    double simple_time = 0, parallel_time = 0, optimized_time = 0;

    for (size_t i = 0; i < n_run; ++i) {
        t0 = oneapi::tbb::tick_count::now();
        simple_copy(col_matrix, continuous_row_matrix, n_rows, n_cols);
        t1 = oneapi::tbb::tick_count::now();
        simple_time += (t1 - t0).seconds();
    }
    std::cout << "Simple copy: " << simple_time / n_run << std::endl;

    for (size_t i = 0; i < n_run; ++i) {
        t0 = oneapi::tbb::tick_count::now();
        parallel_copy(col_matrix, continuous_row_matrix, n_rows, n_cols);
        t1 = oneapi::tbb::tick_count::now();
        parallel_time += (t1 - t0).seconds();
    }
    std::cout << "Parallel copy: " << parallel_time / n_run << std::endl;

    for (size_t i = 0; i < n_run; ++i) {
        t0 = oneapi::tbb::tick_count::now();
        optimized_copy(col_matrix, continuous_row_matrix, n_rows, n_cols);
        t1 = oneapi::tbb::tick_count::now();
        optimized_time += (t1 - t0).seconds();
    }
    std::cout << "Optimized copy: " << optimized_time / n_run << std::endl;

    std::cout << "--------------------------\n";
    std::cout << "Parallel speedup: " << simple_time / parallel_time << std::endl;
    std::cout << "Optimized speedup: " << simple_time / optimized_time << std::endl;

    return 0;
}

void simple_copy(float ** const src, float * const dst, const size_t n_rows, const size_t n_cols) {
    for (size_t i = 0; i < n_rows; ++i) {
        for (size_t j = 0; j < n_cols; ++j) {
            dst[i * n_cols + j] = src[j][i];
        }
    }
}

void parallel_copy(float ** const src, float * const dst, const size_t n_rows, const size_t n_cols) {
    oneapi::tbb::task_arena arena(4);

    const size_t block_size = 256;
    const size_t n_blocks = n_rows / block_size + !!(n_rows % block_size);

    arena.execute([&]{
        oneapi::tbb::parallel_for(oneapi::tbb::blocked_range<size_t>(0, n_blocks, 1), [&](auto& range) {
            for (auto i = range.begin(); i != range.end(); ++i) {
                const size_t start_row = i * block_size;
                const size_t end_row = (start_row + block_size > n_rows) ? n_rows : start_row + block_size;

                for (size_t i = start_row; i < end_row; ++i) {
                    for (size_t j = 0; j < n_cols; ++j) {
                        dst[i * n_cols + j] = src[j][i];
                    }
                }
            }
        });
    });
}

void copy_block(void const * ptr_min, size_t const * offsets, float * dst, const size_t n_rows, const size_t n_cols) {
    float const * ptr_float   = static_cast<float const *>(ptr_min);
    char const * ptr_char = static_cast<char const *>(ptr_min);
    const size_t opt_n_cols = n_cols - n_cols % 8;

    for (size_t i = 0; i < n_rows; ++i) {
        for (size_t j = 0; j < opt_n_cols; j += 8) {
            __m256 ps = _mm512_i64gather_ps(*((__m512i *)&offsets[j]), ptr_float + i, 1);
            _mm256_storeu_ps(dst + i * n_cols + j, ps);
        }
        for (size_t j = opt_n_cols; j < n_cols; ++j) {
            char const * pc = ptr_char + offsets[j];
            dst[i * n_cols + j] = *((float *)(pc) + i);
        }
    }
}

void optimized_copy(float ** const src, float * const dst, const size_t n_rows, const size_t n_cols) {
    oneapi::tbb::task_arena arena(4);

    const size_t block_size = 256;
    const size_t n_blocks = n_rows / block_size + !!(n_rows % block_size);

    float * ptr_min = src[0];
    for (size_t j = 1; j < n_cols; ++j) {
        ptr_min = std::min(ptr_min, src[j]);
    }

    size_t * offsets = new size_t[n_cols];
    for (size_t j = 1; j < n_cols; ++j) {
        offsets[j] = static_cast<size_t>(src[j] - ptr_min);
    }

    arena.execute([&]{
        oneapi::tbb::parallel_for(oneapi::tbb::blocked_range<size_t>(0, n_blocks, 1), [&](auto& range) {
            for (auto i = range.begin(); i != range.end(); ++i) {
                const size_t start_row = i * block_size;
                const size_t local_block_size = (start_row + block_size > n_rows) ? n_rows - start_row: block_size;

                copy_block(static_cast<void*>(ptr_min + start_row), offsets, dst + start_row, local_block_size, n_cols);
            }
        });
    });
}
