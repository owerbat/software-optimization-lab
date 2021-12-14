#include <iostream>

#include <oneapi/tbb/info.h>
#include <oneapi/tbb/parallel_for.h>
#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/task_arena.h>
#include <oneapi/tbb/tick_count.h>

void simple_copy(float ** const src, float * const dst, const size_t n_rows, const size_t n_cols);

int main() {
    constexpr size_t n_rows = 4 * 1024 * 1024;
    constexpr size_t n_cols = 64;

    float ** const col_matrix = new float*[n_cols];
    for (size_t i = 0; i < n_cols; ++i)
        col_matrix[i] = new float[n_rows];

    float * const continuous_row_matrix = new float[n_rows * n_cols];

    oneapi::tbb::tick_count t0 = oneapi::tbb::tick_count::now();
    simple_copy(col_matrix, continuous_row_matrix, n_rows, n_cols);
    oneapi::tbb::tick_count t1 = oneapi::tbb::tick_count::now();
    std::cout << "Simple copy: " << (t1 - t0).seconds() << std::endl;

    return 0;
}

void simple_copy(float ** const src, float * const dst, const size_t n_rows, const size_t n_cols) {
    for (size_t i = 0; i < n_rows; ++i) {
        for (size_t j = 0; j < n_cols; ++j) {
            dst[i * n_cols + j] = src[j][i];
        }
    }
}

void test_tbb() {
    int default_n_threads = oneapi::tbb::info::default_concurrency();
    int max_n_threads = oneapi::tbb::this_task_arena::max_concurrency();
    std::cout << "Default: " << default_n_threads << std::endl;
    std::cout << "Max: " << max_n_threads << std::endl;

    oneapi::tbb::task_arena arena(4);
    arena.execute([]{
        oneapi::tbb::parallel_for(oneapi::tbb::blocked_range<size_t>(0, 10), [&](auto& range) {
            for (auto i = range.begin(); i != range.end(); ++i) {
                std::cout << "Iter #" << i << std::endl << std::flush;
            }
        });
    });
}
