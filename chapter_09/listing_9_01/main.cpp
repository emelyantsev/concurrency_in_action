#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>


#include "thread_pool.h"
#include "common.h"


int main() {

    thread_pool pool;

    constexpr int TASKS = 32;

    std::atomic<int> counter{0};

    auto start = std::chrono::steady_clock::now();

    for (int i = 0; i < TASKS; ++i) {

        pool.submit( [i, &counter] {

            std::this_thread::sleep_for(100ms);

            ++counter;

            // sync_cout
            //     << "task " << i
            //     << " done on thread "
            //     << std::this_thread::get_id()
            //     << "\n";
        });
    }

    while ( counter.load() < TASKS ) {

        std::this_thread::sleep_for( 10ms );
    }

    auto end = std::chrono::steady_clock::now();

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    sync_cout << "\nAll tasks finished\n";
    sync_cout << "Elapsed time: " << elapsed.count() << " ms\n";

    return 0;
}
