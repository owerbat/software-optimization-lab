#include <iostream>

#include <oneapi/tbb/info.h>
#include <oneapi/tbb/parallel_for.h>
#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/task_arena.h>

int main() {
    

    return 0;
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
